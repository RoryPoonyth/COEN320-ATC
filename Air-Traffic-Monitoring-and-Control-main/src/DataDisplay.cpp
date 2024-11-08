#include "DataDisplay.h"
#include "Aircraft.h"

#include <string>
#include <vector>
#include <iostream>
#include <sys/dispatch.h>

using namespace std;

#define ATTACH_POINT "default" // Base attach point for inter-process communication

// Constructor: Initializes DataDisplay with default code value
DataDisplay::DataDisplay() {
    code = -1;
}

// Initializes the listener for operator data display requests
void* DataDisplay::dataDisplayInitializer(void* args) {
	DataDisplay* dataDisplay = (DataDisplay*)args;
	dataDisplay->listen();
	return NULL;
}

// Initializes the listener for aircraft map updates
void* DataDisplay::MapInitializer(void* args) {
	DataDisplay* dataDisplay = (DataDisplay*)args;
	dataDisplay->listenForAircraftMap();
	return NULL;
}

/**
 * Function to continuously listen for updates to the map of aircraft.
 * Updates the internal airspace map based on received aircraft data.
 */
void* DataDisplay::listenForAircraftMap() {
	string airspace[rows][columns];
	initMap(airspace); // Initialize the map to an empty state

	string attachPoint = string(ATTACH_POINT) + "_MAP"; // Attach point for inter-process communication

	name_attach_t *attach;

	// Establish attach point for incoming map data
	if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
		perror("DataDisplay (listenForAircraftMap): Error occurred while creating the attach point");
	}

	vector<AircraftData> aircrafts; // Holds received aircraft data
	AircraftData aircraftCommand; // Temporary storage for incoming commands
	aircrafts.push_back(aircraftCommand);
	aircrafts[0].header.type = 0x05;
	aircrafts[0].header.subtype = 0x05;

	while (true) {
		// Receive messages continuously
		int rcvid = MsgReceive(attach->chid, &aircrafts, sizeof(aircrafts), NULL);

		if (rcvid == -1) { // Error in receiving, exit loop
			break;
		}

		if (rcvid == 0) { // Handle pulse messages
			switch (aircraftCommand.header.code) {
			case _PULSE_CODE_DISCONNECT:
				// Handle client disconnection
				ConnectDetach(aircraftCommand.header.scoid);
				continue;
			case _PULSE_CODE_UNBLOCK:
				// REPLY blocked client wants to unblock
				break;
			default:
				// Ignore other pulses
				break;
			}
			continue;
		}

		// Handle connection messages from name_open()
		if (aircraftCommand.header.type == _IO_CONNECT ) {
			MsgReply(rcvid, EOK, NULL, 0);
			continue;
		}

		// Reject unhandled QNX IO messages
		if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX ) {
			MsgError(rcvid, ENOSYS);
			continue;
		}

		// If aircraft data type and subtype are correct, update the map
		if (aircrafts[0].header.type == 0x05 && aircrafts[0].header.subtype == 0x05) {
			vector<Aircraft*> receivedAircrafts;

			// Process each incoming aircraft's data
			for (AircraftData aircraftCommand : aircrafts) {
				Aircraft* aircraft = new Aircraft();
				aircraft->setFlightId(aircraftCommand.flightId);
				aircraft->setPositionX(aircraftCommand.positionX);
				aircraft->setPositionY(aircraftCommand.positionY);
				aircraft->setPositionZ(aircraftCommand.positionZ);
				aircraft->setSpeedX(aircraftCommand.speedX);
				aircraft->setSpeedY(aircraftCommand.speedY);
				aircraft->setSpeedZ(aircraftCommand.speedZ);

				receivedAircrafts.push_back(aircraft);
			}

			updateMap(receivedAircrafts, airspace); // Update the display with new aircraft positions

			// Acknowledge receipt of message to CommandSystem
			MsgReply(rcvid, EOK, NULL, 0);

			receivedAircrafts.clear(); // Clear data after processing
			continue; // Listen for the next request
		}
	}

	return EXIT_SUCCESS;
}

/**
 * Function to listen and display specific aircraft info based on operator requests.
 */
void* DataDisplay::listen() {
	string attachPoint = string(ATTACH_POINT) + "_datadisplay_"; // Attach point for specific data display requests

	name_attach_t *attach;

	// Establish the attach point for incoming data display requests
	if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
		perror("DataDisplay (listen): Error occurred while creating the attach point");
	}

	AircraftData aircraftCommand; // Holds data received from operator requests

	while (true) {
		int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

		if (rcvid == -1) { // Error in receiving, exit loop
			break;
		}

		if (rcvid == 0) { // Handle pulse messages
			switch (aircraftCommand.header.code) {
			case _PULSE_CODE_DISCONNECT:
				// Handle client disconnection
				ConnectDetach(aircraftCommand.header.scoid);
				continue;
			case _PULSE_CODE_UNBLOCK:
				// REPLY blocked client wants to unblock
				break;
			default:
				// Ignore other pulses
				break;
			}
			continue;
		}

		// Handle connection messages from name_open()
		if (aircraftCommand.header.type == _IO_CONNECT) {
			MsgReply(rcvid, EOK, NULL, 0);
			continue;
		}

		// Reject unhandled QNX IO messages
		if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX) {
			MsgError(rcvid, ENOSYS);
			continue;
		}

		// Process operator requests to display specific aircraft information
		if (aircraftCommand.header.type == 0x05 && aircraftCommand.header.subtype == 0x01) {
			cout << endl << "-----------------------------------------------------------------------------" << endl
				 << "(Operator Request) Received aircraft data: flightId = " << aircraftCommand.flightId
				 << ", PositionX = " << aircraftCommand.positionX
				 << ", PositionY = " << aircraftCommand.positionY
				 << ", PositionZ = " << aircraftCommand.positionZ
				 << ", SpeedX = " << aircraftCommand.speedX
				 << ", SpeedY = " << aircraftCommand.speedY
				 << ", SpeedZ = " << aircraftCommand.speedZ
				 << endl << "-----------------------------------------------------------------------------" << endl;

			// Acknowledge receipt of message
			MsgReply(rcvid, EOK, NULL, 0);
			continue; // Listen for the next request
		}
	}

	return EXIT_SUCCESS;
}

/**
 * -------------------------------------------------------------------------------------------------
 * MAP FUNCTIONS
 * -------------------------------------------------------------------------------------------------
 */

/**
 * Initialize the airspace map with empty cells.
 */
void DataDisplay::initMap(string (&airspace)[rows][columns]) {
	for(int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			airspace[i][j] = " [  ] ";
		}
	}
}

/**
 * Clear the previous aircraft positions from the map.
 */
void DataDisplay::clearPrevious(string (&airspace)[rows][columns], int indexI, int indexJ, string flightId) {
	for(int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			if(airspace[i][j] == flightId) {
				airspace[i][j] = " [  ] "; // Clear cell if it matches the flightId
			}
		}
	}
}

/**
 * Write the current map state to a log file.
 */
void DataDisplay::writeMap(string mapAsString) {
	code = creat("/data/home/qnxuser/MapLog.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if(code == -1) {
		cout << "Map could not be logged, error" << endl;
	} else {
		// Buffer the map data to write to the log file
		char *blockBuffer = new char[mapAsString.length() + 1];
		sprintf(blockBuffer, "%s", mapAsString.c_str());
		write(code, blockBuffer, mapAsString.length() + 1);
		write(code, "====", 4);
		write(code, "\n", 1);
		delete[] blockBuffer;
		close(code);
	}
}

/**
 * Update the map based on current aircraft positions.
 */
string DataDisplay::updateMap(vector<Aircraft*>& planes, string (&airspace)[rows][columns]) {
	cout << endl;

	// Each cell represents a 1000 ft area
	for(int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			for(size_t k = 0; k < planes.size(); k++) {
				int xCurrent = planes[k]->getPositionX();
				int yCurrent = planes[k]->getPositionY();

				// Fit aircraft in 1000 sq. ft cells
				xCurrent /= 1000;
				yCurrent /= 1000;
				try {
					if(i == yCurrent && j == xCurrent && airspace[i][j] == " [  ] ") {
						clearPrevious(airspace, i, j, " [ " + to_string(planes[k]->getFlightId()) + " ] ");
						airspace[i][j] = " [ " + to_string(planes[k]->getFlightId()) + " ] ";
					}
				} catch(...) {
					// Handle out-of-bounds exceptions gracefully
				}
			}
		}
	}

	// Convert the map to a string and print to console
	string mapAsString = "";
	for(int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			cout << airspace[i][j];
			mapAsString += airspace[i][j];
		}
		cout << endl;
		mapAsString += "\n";
	}
	writeMap(mapAsString);
	return mapAsString;
}

// Destructor to clean up resources, if necessary
DataDisplay::~DataDisplay() {
}
