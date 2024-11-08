#include "Radar.h"
#include "Aircraft.h"

#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <sys/dispatch.h>

using namespace std;

#define ATTACH_POINT "default" // Define the base attach point for communication

// Default constructor for Radar class
Radar::Radar() {
}

// Constructor that initializes the Radar with a list of Aircraft objects
Radar::Radar(vector<Aircraft> &aircrafts) {
	this->aircrafts = aircrafts;
}

// Constructor that initializes the Radar with a single Aircraft object
Radar::Radar(Aircraft &aircraft) {
	this->aircraft = aircraft;
}

// Static initializer function to create a thread to start pinging aircraft
void* Radar::radarInitializer(void* args) {
	Radar* radar = (Radar*)args;
	radar->pingAircraft(); // Start pinging aircraft to get updates
	return NULL;
}

/**
 * Continuously pings aircraft to request their current position and speed.
 */
void* Radar::pingAircraft() {
	string attachPoint = string(ATTACH_POINT) + "_RADAR"; // Set the attach point for radar

	name_attach_t *attach;

	// Create an attach point for communication
	if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
		perror("Radar (pingAircraft): Error occurred while creating the attach point");
	}

	AircraftData aircraftCommand; // Struct to hold incoming aircraft data
	while (true) {
		int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL); // Wait for message

		if (rcvid == -1) { // Error condition, exit loop
			break;
		}

		// Handle pulse messages received with specific codes
		if (rcvid == 0) { // Pulse received
			switch (aircraftCommand.header.code) {
			case _PULSE_CODE_DISCONNECT:
				// Handle client disconnection
				ConnectDetach(aircraftCommand.header.scoid);
				continue;
			case _PULSE_CODE_UNBLOCK:
				// Handle client unblock request
				break;
			default:
				// Handle other pulse codes
				break;
			}
			continue;
		}

		// Handle connection message from name_open() by acknowledging it
		if (aircraftCommand.header.type == _IO_CONNECT) {
			MsgReply(rcvid, EOK, NULL, 0);
			continue;
		}

		// Reject any unhandled QNX IO messages
		if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX) {
			MsgError(rcvid, ENOSYS);
			continue;
		}

		// Process radar data request if type and subtype match expected values
		if (aircraftCommand.header.type == 0x08) {
			if (aircraftCommand.header.subtype == 0x08) {
				string attachPoint = string(ATTACH_POINT) + "_" + to_string(aircraftCommand.flightId); // Unique attach point

				// Establish communication with the aircraft
				int server_coid;
				if ((server_coid = name_open(attachPoint.c_str(), 0)) == -1) {
					perror("Error occurred while attaching the channel");
				}

				// Set the command type and subtype to request aircraft data (speed and position)
				aircraftCommand.header.type = 0x00;
				aircraftCommand.header.subtype = 0x01;

				// Send data and expect a response with position/velocity data
				if (MsgSend(server_coid, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
					cout << "Error while sending the message for aircraft " << ": " << strerror(errno) << endl;
					name_close(server_coid);
				}

				// Reply to ComputerSystem with the received aircraft information
				MsgReply(rcvid, EOK, &aircraftCommand, sizeof(aircraftCommand));

				// Continue listening for further requests
				continue;
			} else {
				MsgError(rcvid, ENOSYS);
				continue;
			}
		}
	}

	return EXIT_SUCCESS;
}

/**
 * Requests the position and speed of a specific aircraft based on its flight ID.
 * @param flightId - The unique identifier of the aircraft
 * @return AircraftData - The data structure containing the aircraft's current position and speed
 */
AircraftData Radar::operatorRequestPingAircraft(int flightId) {
	string attachPoint = string(ATTACH_POINT) + "_" + to_string(flightId); // Unique attach point for aircraft

	// Establish communication with the aircraft
	int server_coid;
	if ((server_coid = name_open(attachPoint.c_str(), 0)) == -1) {
		perror("Error occurred while attaching the channel");
	}

	// Set command type and subtype to request aircraft data
	AircraftData aircraftCommand;
	aircraftCommand.header.type = 0x00;
	aircraftCommand.header.subtype = 0x01;

	// Send request and expect a response with position/velocity data
	if (MsgSend(server_coid, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
		cout << "Error while sending the message for aircraft " << ": " << strerror(errno) << endl;
		name_close(server_coid);
	}

	return aircraftCommand;
}

// Destructor for Radar class to clean up resources if necessary
Radar::~Radar() {
}
