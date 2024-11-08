#include "ComputerSystem.h"
#include "CommunicationSystem.h"
#include "Radar.h"
#include "Aircraft.h"
#include "ctimer.h"
#include <sys/dispatch.h>
#include <vector>
#include <string>

#define ATTACH_POINT "default" // Define the base attach point for communication

using namespace std;

// Constructor to initialize the ComputerSystem object
ComputerSystem::ComputerSystem() {
}

// Sets the list of Aircraft objects that this ComputerSystem will manage
void ComputerSystem::setAircrafts(std::vector<Aircraft*> aircrafts) {
	this->aircrafts = aircrafts;
}

// Retrieves an Aircraft object based on its flight ID
Aircraft* ComputerSystem::getAircraftByFlightId(int flightId) {
	for (Aircraft* aircraft : aircrafts) {
		if (aircraft->getFlightId() == flightId) {
			return aircraft;
		}
	}
	return nullptr; // Return nullptr if no aircraft matches the flight ID
}

// Static function to start the map display thread
void* ComputerSystem::MapDisplayThread(void* args) {
	ComputerSystem* computerSystem = (ComputerSystem*)args;
	computerSystem->MapDisplay();
	return NULL;
}

// Static function to start the computer system operations thread
void* ComputerSystem::computerSystemThread(void* args) {
	ComputerSystem* computerSystem = (ComputerSystem*)args;
	computerSystem->computerSystemOperations();
	return NULL;
}

// Static function to start the separation check thread
void* ComputerSystem::separationCheckThread(void* args){
	ComputerSystem* computerSystem = (ComputerSystem*)args;
	computerSystem->separationCheck();
	return NULL;
}

/**
 * Function to display a 2D map of aircraft positions, updating every 5 seconds.
 */
void* ComputerSystem::MapDisplay() {
	while (true) { // Infinite loop with a 5-second delay between each iteration
		vector<AircraftData> aircraftsInfo; // Vector to hold data of all aircrafts
		Radar radar; // Radar object to get data from each aircraft

		// Vector to hold threads for each aircraft's listener
		vector<pthread_t> threads;

		// Create listener threads for each aircraft
		for (size_t i = 0; i < aircrafts.size(); i++) {
			string attachPointRAD = string(ATTACH_POINT) + "_" + to_string(aircrafts[i]->getFlightId());
			pthread_t thread;
			AircraftListenArgs* args = new AircraftListenArgs{aircrafts[i], attachPointRAD};
			pthread_create(&thread, NULL, &Aircraft::aircraftListenHelper, args);
			threads.push_back(thread); // Store thread for later cleanup
		}

		// Retrieve aircraft data from Radar and store in aircraftsInfo
		for (size_t i = 0; i < aircrafts.size(); i++) {
			AircraftData aircraftResult = radar.operatorRequestPingAircraft(aircrafts[i]->getFlightId());
			aircraftsInfo.push_back(aircraftResult);
		}

		// Clean up listener threads created during this period
		for (pthread_t thread : threads) {
			pthread_cancel(thread);
		}

		// Set header type and subtype for communication with DataDisplay
		aircraftsInfo[0].header.type = 0x05;
		aircraftsInfo[0].header.subtype = 0x05;

		string attachPointDataDisplay = string(ATTACH_POINT) + "_MAP"; // Attach point for DataDisplay

		// Establish connection with DataDisplay
		int coid;
		if ((coid = name_open(attachPointDataDisplay.c_str(), 0)) == -1) {
			perror("ComputerSystem (MapDisplay): Error occurred while attaching the channel");
			return (void*)EXIT_FAILURE;
		}

		// Send aircraft data to DataDisplay, expect no response
		if (MsgSend(coid, &aircraftsInfo, sizeof(aircraftsInfo), NULL, 0) == -1) {
			cout << "Error while sending the message: " << strerror(errno) << endl;
			name_close(coid);
			return (void*)EXIT_FAILURE;
		}

		sleep(5); // Wait for 5 seconds before updating the map again
	}

	return EXIT_SUCCESS;
}

/**
 * Handles Computer System operations based on requests from OperatorConsole:
 * 1. Update Aircraft Speed (via CommunicationSystem)
 * 2. Retrieve Aircraft Information (via Radar)
 */
void ComputerSystem::computerSystemOperations() {
	name_attach_t *attach;

	// Create attach point for receiving messages
	if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
		perror("Error occurred while creating the attach point sendNewAircraftSpeedToCommunicationSystem");
	}

	int newSpeedX, newSpeedY, newSpeedZ; // Variables for new speed values
	AircraftData aircraftCommand; // Struct to hold incoming aircraft commands

	while (true) {
		int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

		if (rcvid == -1) { // Error condition, exit loop
			break;
		}

		/**
		 * Update Aircraft Speed (from OperatorConsole request)
		 */
		if (aircraftCommand.header.type == 0x02 && aircraftCommand.header.subtype == 0x00) {
			newSpeedX = aircraftCommand.speedX;
			newSpeedY = aircraftCommand.speedY;
			newSpeedZ = aircraftCommand.speedZ;
			MsgReply(rcvid, EOK, NULL, 0); // Acknowledge the request

			// Retrieve the specified aircraft by flight ID
			Aircraft* aircraft = getAircraftByFlightId(aircraftCommand.flightId);
			if (aircraft == nullptr) {
				cout << "Error: Aircraft with flight ID " << aircraftCommand.flightId << " was not FOUND." << endl;
				continue;
			}

			// Start listener thread and update speed via CommunicationSystem
			string attachPoint = string(ATTACH_POINT) + "_" + to_string(aircraftCommand.flightId);
			pthread_t thread;
			AircraftListenArgs* args = new AircraftListenArgs{aircraft, attachPoint};
			pthread_create(&thread, NULL, &Aircraft::aircraftListenHelper, args);

			CommunicationSystem communicationSystem;
			communicationSystem.relayNewSpeed(*aircraft, newSpeedX, newSpeedY, newSpeedZ);
			alarm();
			continue;

		/**
		 * Retrieve Aircraft Information (from OperatorConsole request)
		 */
		} else if (aircraftCommand.header.type == 0x02 && aircraftCommand.header.subtype == 0x01) {
			MsgReply(rcvid, EOK, NULL, 0); // Acknowledge the request

			// Retrieve specified aircraft
			Aircraft* aircraft = getAircraftByFlightId(aircraftCommand.flightId);
			if (aircraft == nullptr) {
				cout << "Error: Aircraft with flight ID " << aircraftCommand.flightId << " was not found." << endl;
				continue;
			}

			// Start listener thread for aircraft
			string attachPointRAD = string(ATTACH_POINT) + "_" + to_string(aircraftCommand.flightId);
			pthread_t thread;
			AircraftListenArgs* args = new AircraftListenArgs{aircraft, attachPointRAD};
			pthread_create(&thread, NULL, &Aircraft::aircraftListenHelper, args);

			// Set up connection with Radar
			string radarAttachPoint  = string(ATTACH_POINT) + "_RADAR";
			int radarCOID;
			if ((radarCOID = name_open(radarAttachPoint.c_str(), 0)) == -1) {
				perror("Computer System (computerSystemOperations): Error occurred while attaching the channel for Radar Request");
				return;
			}

			// Prepare Radar request and expect response with aircraft info
			aircraftCommand.header.type = 0x08;
			aircraftCommand.header.subtype = 0x08;

			if (MsgSend(radarCOID, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
				cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
				name_close(radarCOID);
			}

			// Send the received data to DataDisplay
			string attachPointDATADISPLAY = string(ATTACH_POINT) + "_datadisplay_";
			int coid;
			if ((coid = name_open(attachPointDATADISPLAY.c_str(), 0)) == -1) {
				perror("Error occurred while attaching the channel IN COMPSYS FOR DATADISPLAY");
				return;
			}

			// Send data to DataDisplay with the received info
			aircraftCommand.header.type = 0x05;
			aircraftCommand.header.subtype = 0x01;

			if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
				name_close(coid);
				return;
			}

			continue;
		}
	}
	name_detach(attach, 0); // Clean up the attach point
}

/**
 * Continuously checks the separation distance between aircraft and ensures safety limits.
 */
void ComputerSystem::separationCheck() {
	name_attach_t *attach;
	string attachPoint = string(ATTACH_POINT) + "_separation";

	if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
		perror("Error occurred while creating the attach point separation");
	}

	int n = 180; // Number of seconds to predict into the future
	int p = 5; // Interval for checking separation
	SeparationData separationCommand;
	cTimer timer(p, 0);

	while (true) {
		alarm();
		int rcvid;
		for (int i = 0; i < 2; i++) { // Attempt to receive a message up to 2 times
			if ((rcvid = MsgReceive(attach->chid, &separationCommand, sizeof(separationCommand), NULL)) != -1) {
				n = separationCommand.n_seconds;
				timer.setTimerSpec(separationCommand.p_interval, 0);
				cout << "Message Received N: " << n << " P:" << separationCommand.p_interval << endl;
				MsgReply(rcvid, EOK, NULL, 0);
				i = 200; // Exit loop after receiving a message
			}
		}

		// Define separation constraint dimensions
		string output = "";
		int width = 3000;
		int height = 1000;

		for (Aircraft* a : aircrafts) {
			int x = a->getPositionX() + n * a->getSpeedX();
			int y = a->getPositionY() + n * a->getSpeedY();
			int z = a->getPositionZ() + n * a->getSpeedZ();

			for (Aircraft* b : aircrafts) {
				if (a != b) {
					int bx = b->getPositionX() + n * b->getSpeedX();
					int by = b->getPositionY() + n * b->getSpeedY();
					int bz = b->getPositionZ() + n * b->getSpeedZ();

					// Check separation constraints
					if (bx > x + width || bx < x - width || by > y + width || by < y - width || bz > z + height || bz < z - height) {
						continue;
					}

					// Append message if separation constraint violation is detected
					output.append("" + std::to_string(a->getFlightId()) + " close to " + std::to_string(b->getFlightId()) + "\n");
				}
			}
		}

		if (output != "") {
			cout << "SEPARATION CONSTRAINT VIOLATION AT CURRENT TIME + " << n << " seconds " << endl;
			cout << output;
		}

		sleep(p);
	}
	name_detach(attach, 0); // Clean up attach point
}

/**
 * Alarm function to monitor and trigger alerts based on future safety checks.
 */
void ComputerSystem::alarm() {
	int m = 180; // Duration to check into the future in seconds

	string output = "";
	int width = 3000;
	int height = 1000;

	for (int n = 0; n < m; n++) { // Iterate every second up to 180 seconds
		for (Aircraft* a : aircrafts) {
			int x = a->getPositionX() + n * a->getSpeedX();
			int y = a->getPositionY() + n * a->getSpeedY();
			int z = a->getPositionZ() + n * a->getSpeedZ();

			for (Aircraft* b : aircrafts) {
				if (a != b) {
					int bx = b->getPositionX() + n * b->getSpeedX();
					int by = b->getPositionY() + n * b->getSpeedY();
					int bz = b->getPositionZ() + n * b->getSpeedZ();

					// Check if separation is violated
					if (bx > x + width || bx < x - width || by > y + width || by < y - width || bz > z + height || bz < z - height) {
						continue;
					}

					output.append("" + std::to_string(a->getFlightId()) + " close to " + std::to_string(b->getFlightId()) + "\n");
					n = m; // End loop after finding a violation
				}
			}
		}

		if (output != "") {
			cout << "Alarm: safety violation will happen within 3 minutes" << endl;
			cout << output;
		}
	}
}

// Destructor for ComputerSystem to clean up resources if necessary
ComputerSystem::~ComputerSystem() {
}
