#include "CommunicationSystem.h"
#include "Aircraft.h"

#include <string>
#include <sys/dispatch.h>

using namespace std;

#define ATTACH_POINT "default" // Defines the base attach point for communication

// Constructor for CommunicationSystem class, initializes the communication system
CommunicationSystem::CommunicationSystem() {
}

/**
 * Relay new speed values to a specific Aircraft based on flightId.
 * This function attempts to establish a communication channel with the target aircraft
 * and sends updated speed values for the x, y, and z axes.
 *
 * @param R Reference to the Aircraft object to relay new speed values to.
 * @param newSpeedX New speed value for the x-axis.
 * @param newSpeedY New speed value for the y-axis.
 * @param newSpeedZ New speed value for the z-axis.
 * @return A void pointer to indicate success or failure (EXIT_SUCCESS or EXIT_FAILURE).
 */
void* CommunicationSystem::relayNewSpeed(Aircraft &R, int newSpeedX, int newSpeedY, int newSpeedZ) {
	// Generate a unique attach point name by appending the flight ID to the base attach point
	string attachPoint = string(ATTACH_POINT) + "_" + to_string(R.getFlightId());

	int attempts = 0; // Counter for attempts to establish a connection

	// Try up to 3 attempts to open a communication channel
	while (attempts < 3) {
		// Attempt to open the communication channel with the target aircraft
		int id = name_open(attachPoint.c_str(), 0);
		if (id != -1) { // If the channel opens successfully
			// Create and configure the AircraftData command message
			AircraftData aircraftCommand;
			aircraftCommand.header.type = 0x00; // Type to specify a command
			aircraftCommand.header.subtype = 0x02; // Subtype for a speed update command
			aircraftCommand.speedX = newSpeedX; // Set new speed for x-axis
			aircraftCommand.speedY = newSpeedY; // Set new speed for y-axis
			aircraftCommand.speedZ = newSpeedZ; // Set new speed for z-axis

			// Send the new speed data with no expected response
			if (MsgSend(id, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
				cout << "Error while sending new speed data: " << strerror(errno) << endl;
				name_close(id); // Close the channel in case of an error
				return (void*)EXIT_FAILURE; // Return failure
			}

			name_close(id); // Close the channel after successful transmission
			return EXIT_SUCCESS; // Return success
		} else {
			// If opening the channel fails, increment attempt counter and retry
			attempts++;
		}
	}

	// If all attempts fail, print error message and return failure
	cout << "ERROR: Failed to attach the channel." << endl;
	return (void*)EXIT_FAILURE;
}

// Destructor for CommunicationSystem class, cleans up resources (if any)
CommunicationSystem::~CommunicationSystem() {
}
