#include "CommunicationSystem.h"
#include "Aircraft.h"

#include <iostream>
#include <string>
#include <sys/dispatch.h>
#include <cerrno> // for errno
#include <cstring> // for strerror

#define ATTACH_POINT "default"

CommunicationSystem::CommunicationSystem() {}

/**
 * Relay new speed to the specified aircraft.
 * @param aircraft The Aircraft to which speed update is to be relayed.
 * @param newSpeedX New X speed for the aircraft.
 * @param newSpeedY New Y speed for the aircraft.
 * @param newSpeedZ New Z speed for the aircraft.
 * @return true if the message was sent successfully, false otherwise.
 */
bool CommunicationSystem::relayNewSpeed(Aircraft& aircraft, int newSpeedX, int newSpeedY, int newSpeedZ) {
    std::string attachPoint = std::string(ATTACH_POINT) + "_" + std::to_string(aircraft.getFlightId());

    int attempts = 0;
    constexpr int max_attempts = 3;
    while (attempts < max_attempts) {
        // Attempt to open the communication channel
        int id = name_open(attachPoint.c_str(), 0);
        if (id != -1) {
            // Prepare the command to update aircraft speed
            AircraftData aircraftCommand{};
            aircraftCommand.header.type = 0x00;
            aircraftCommand.header.subtype = 0x02;
            aircraftCommand.speedX = newSpeedX;
            aircraftCommand.speedY = newSpeedY;
            aircraftCommand.speedZ = newSpeedZ;

            // Send the command
            if (MsgSend(id, &aircraftCommand, sizeof(aircraftCommand), nullptr, 0) == -1) {
                std::cerr << "Error sending new speed data: " << strerror(errno) << std::endl;
                name_close(id);
                return false;
            }

            name_close(id);
            return true;
        } else {
            ++attempts;
            std::cerr << "Attempt " << attempts << " to open channel failed." << std::endl;
        }
    }

    std::cerr << "ERROR: Failed to open communication channel after " << max_attempts << " attempts." << std::endl;
    return false;
}

CommunicationSystem::~CommunicationSystem() {}
