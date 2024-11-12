#include "OperatorConsole.h"
#include "ComputerSystem.h"
#include "CommunicationSystem.h"
#include "Aircraft.h"

#include <sys/dispatch.h>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <iostream>

#define ATTACH_POINT "default" // Define the base attach point for communication

using namespace std;

// Constructor to initialize the OperatorConsole object
OperatorConsole::OperatorConsole() {
}

// Logs operator commands to a file
void OperatorConsole::writeLog(string log_entry) {
    int code = open("/data/home/qnxuser/OperatorLog.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (code == -1) {
        std::cerr << "Operator commands could not be logged, error" << std::endl;
    } else {
        // Write log entry and newline
        ssize_t bytes_written = write(code, log_entry.c_str(), log_entry.size());
        if (bytes_written == -1) {
            std::cerr << "Error writing to log file" << std::endl;
        }

        // Write newline separately
        bytes_written = write(code, "\n", 1);
        if (bytes_written == -1) {
            std::cerr << "Error writing newline to log file" << std::endl;
        }

        close(code); // Close the file after writing
    }
}

// Initializes a thread to listen for operator input
void* OperatorConsole::listenForOperatorInput(void* args) {
	OperatorConsole* operatorConsole = (OperatorConsole*)args;
	operatorConsole->listenForUserInput();
	return NULL;
}

// Initializes a thread to update an aircraft's speed
void* OperatorConsole::updateAircraftSpeedThread(void* args) {
	OperatorConsole* operatorConsole = (OperatorConsole*)args;
	operatorConsole->updateAircraftSpeed();
	return NULL;
}

// Initializes a thread to display aircraft data
void* OperatorConsole::displayAircraftDataThread(void* args) {
	OperatorConsole* operatorConsole = (OperatorConsole*)args;
	operatorConsole->displayAircraftData();
	return NULL;
}

/**
 * Listen for user input to execute specific commands such as displaying aircraft info,
 * updating aircraft speed, or adjusting separation constraints.
 */
void OperatorConsole::listenForUserInput() {
	while (true) { // Continuously listen for user commands
		int commandType; // Store the type of command
		int flightId; // Store the flight ID

		// Prompt the user for input
		cout << endl << "Enter your choice... (1 to display an aircraft's info, 2 to update an aircraft's speed, 3 for separation constraint)" << endl;
		cin >> commandType;

		// Command to display aircraft info
        if (commandType == 1) {
        	cout << endl << "Enter Flight Id: ";
			cin >> flightId;
        	log_entry += "display flight info for flight #" + std::to_string(flightId);
        	writeLog(log_entry); // Log the command
        	log_entry = "";

        	// Start a thread to display aircraft data
        	pthread_t displayDataThread;
            pthread_create(&displayDataThread, nullptr, &OperatorConsole::displayAircraftDataThread, this);

			AircraftData aircraftCommand;
			aircraftCommand.header.type = 0x04;
			aircraftCommand.header.subtype = 0x02;
			aircraftCommand.flightId = flightId;

			sleep(1); // Delay to allow the system to create/attach channels

			// Generate the unique attach point name
			string attachPoint = string(ATTACH_POINT) + "info";

			// Establish communication
			int coid;
			if ((coid = name_open(attachPoint.c_str(), 0)) == -1) {
				perror("Error occurred while attaching the channel listenForUserInput FUNCTION [1]");
				return;
			}

			// Send data expecting no response
			if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
				name_close(coid);
				return;
			}

		// Command to update aircraft speed
		} else if (commandType == 2) {
			cout << endl << "Enter Flight Id: ";
			cin >> flightId;
        	log_entry += "Change flight speed for flight #" + std::to_string(flightId) + "\nNew speeds (X,Y,Z) = (";

			// Start a thread to update aircraft speed
			pthread_t updateAircraftSpeedThread;
			pthread_create(&updateAircraftSpeedThread, nullptr, &OperatorConsole::updateAircraftSpeedThread, this);

			int newSpeedX, newSpeedY, newSpeedZ;

			// Get new speeds from user
			cout << endl << "Enter SpeedX: ";
			cin >> newSpeedX;
            log_entry += std::to_string(newSpeedX) + ",";
			cout << endl << "Enter SpeedY: ";
			cin >> newSpeedY;
            log_entry += std::to_string(newSpeedY) + ",";
			cout << endl << "Enter SpeedZ: ";
			cin >> newSpeedZ;
            log_entry += std::to_string(newSpeedZ) + ")";
            writeLog(log_entry); // Log the command
            log_entry = "";

			AircraftData aircraftCommand;
			aircraftCommand.header.type = 0x04;
			aircraftCommand.header.subtype = 0x01;
			aircraftCommand.flightId = flightId;
			aircraftCommand.speedX = newSpeedX;
			aircraftCommand.speedY = newSpeedY;
			aircraftCommand.speedZ = newSpeedZ;

			// Generate the unique attach point name
			string attachPoint = string(ATTACH_POINT) + "inner_transfer";

			// Establish communication
			int coid;
			if ((coid = name_open(attachPoint.c_str(), 0)) == -1) {
				perror("Error occurred while attaching the channel listenForUserInput FUNCTION [2]");
				return;
			}

			// Send data expecting no response
			if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
				name_close(coid);
				return;
			}

		// Command to update separation constraints
		} else if (commandType == 3) {
			log_entry += "Change input N and interval P at runtime for separation constraint\nNew input N and P: ";

			// Get new N and P values from user
			int n;
			cout << endl << "Enter N seconds for separation constraint: ";
			cin >> n;
			log_entry += std::to_string(n) + ", ";
			int p;
			cout << endl << "Enter interval for separation constraint: ";
			cin >> p;
			log_entry += std::to_string(p) + "\n";
			writeLog(log_entry); // Log the command
			log_entry = "";

			// Generate the unique attach point name
			string attachPoint = string(ATTACH_POINT) + "_separation";

			// Establish communication
			int coid;
			if ((coid = name_open(attachPoint.c_str(), 0)) == -1) {
				perror("Error occurred while attaching the channel listenForUserInput FUNCTION [3]");
				return;
			}

			SeparationData separationCommand;
			separationCommand.n_seconds = n;
			separationCommand.p_interval = p;

			// Send data expecting no response
			if (MsgSend(coid, &separationCommand, sizeof(separationCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for computer system: " << strerror(errno) << endl;
				name_close(coid);
				return;
			}
		} else {
			cout << "Invalid command type." << endl; // Invalid command input
		}
	}
}

// Updates the speed of an aircraft based on received commands
void* OperatorConsole::updateAircraftSpeed() {
	name_attach_t *attach;

	// Generate the unique attach point name
	string attachPointInner = string(ATTACH_POINT) + "inner_transfer";

	// Create an attach point for the update command
	if ((attach = name_attach(NULL, attachPointInner.c_str(), 0)) == NULL) {
		perror("Error occurred while creating the attach point updateAircraftSpeed");
		return (void*)EXIT_FAILURE;
	}

	AircraftData aircraftCommand;
	while (true) {
		int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

		if (rcvid == -1) { // Error condition
			break;
		}

		// Update speed if command type matches expected values
		if (aircraftCommand.header.type == 0x04 && aircraftCommand.header.subtype == 0x01) {
			MsgReply(rcvid, EOK, NULL, 0);
			name_detach(attach, 0);

			// Establish connection for the update
			int coid;
			if ((coid = name_open(ATTACH_POINT, 0)) == -1) {
				perror("Error occurred while attaching the channel UPDATEAIRCRAFTSPEED FUNCTION");
				return (void*)EXIT_FAILURE;
			}

			aircraftCommand.header.type = 0x02;
			aircraftCommand.header.subtype = 0x00;

			// Send updated speed command expecting no response
			if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for aircraft CSOP " << ": " << strerror(errno) << endl;
				name_close(coid);
				return (void*)EXIT_FAILURE;
			}

			break;
		}
	}

	return EXIT_SUCCESS;
}

// Displays specific aircraft data based on received commands
void* OperatorConsole::displayAircraftData() {
	name_attach_t *attach;

	// Generate the unique attach point name
	string attachPointInner = string(ATTACH_POINT) + "info";

	// Create an attach point for the data display command
	if ((attach = name_attach(NULL, attachPointInner.c_str(), 0)) == NULL) {
		perror("Error occurred while creating the attach point displayAircraftData");
		return (void*)EXIT_FAILURE;
	}

	AircraftData aircraftCommand;
	while (true) {
		int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

		if (rcvid == -1) { // Error condition
			break;
		}

		// Display data if command type matches expected values
		if (aircraftCommand.header.type == 0x04 && aircraftCommand.header.subtype == 0x02) {
			int coid;
			if ((coid = name_open(ATTACH_POINT, 0)) == -1) {
				perror("Error occurred while attaching the channel DISPLAYAIRCRAFTDATA FUNCTION");
				return (void*)EXIT_FAILURE;
			}

			aircraftCommand.header.type = 0x02;
			aircraftCommand.header.subtype = 0x01;

			// Send aircraft data expecting no response
			if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
				cout << "Error while sending the message for aircraft CSOP " << ": " << strerror(errno) << endl;
				name_close(coid);
				return (void*)EXIT_FAILURE;
			}

			MsgReply(rcvid, EOK, NULL, 0);
			name_detach(attach, 0);

			break;
		}
	}

	return EXIT_SUCCESS;
}

// Destructor: Cleans up resources (if any)
OperatorConsole::~OperatorConsole() {
}
