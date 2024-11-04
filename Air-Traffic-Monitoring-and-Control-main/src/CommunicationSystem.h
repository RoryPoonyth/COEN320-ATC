#pragma once

#include "Aircraft.h"
#include <string>

class CommunicationSystem {
public:
    CommunicationSystem();
    virtual ~CommunicationSystem();

    // Functions to relay new data to the specified aircraft
    bool relayNewSpeed(Aircraft& aircraft, int newSpeedX, int newSpeedY, int newSpeedZ);
    bool relayNewPosition(Aircraft& aircraft, int newPositionX, int newPositionY, int newPositionZ);
};
