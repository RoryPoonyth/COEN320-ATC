#include "CommunicationSystem.h"
#include "Aircraft.h"

#include <string>
#include <sys/dispatch.h>

using namespace std;

#define ATTACH_POINT "default"

CommunicationSystem::CommunicationSystem() {}

/**
 * Relay new speed to the aircraft specified by Operator Request
 */
void* CommunicationSystem::relayNewSpeed(Aircraft &aircraft, int newSpeedX, int newSpeedY, int newSpeedZ) {
    // Generate the unique attach point name based on flightId
    string attachPoint = string(ATTACH_POINT) + "_" + to_string(aircraft.getFlightId());

    int attempts = 0;
    while (attempts < 3) {
        // Attempt to open the communication channel
        int id = name_open(attachPoint.c_str(), 0);
        if (id != -1) {
            // Set header type and subtype to request speed update
            AircraftData aircraftCommand;
            aircraftCommand.header.type = 0x00;
            aircraftCommand.header.subtype = 0x02;
            aircraftCommand.speedX = newSpeedX;
            aircraftCommand.speedY = newSpeedY;
            aircraftCommand.speedZ = newSpeedZ;

            // Send new speed expecting no response
            if (MsgSend(id, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
                cout << "Error while sending new speed data: " << strerror(errno) << endl;
                name_close(id);
                return (void*)EXIT_FAILURE;
            }

            name_close(id);
            return EXIT_SUCCESS;
        } else {
            attempts++;
        }
    }

    cout << "ERROR: Failed to attach the channel." << endl;
    return (void*)EXIT_FAILURE;
}

CommunicationSystem::~CommunicationSystem() {}
