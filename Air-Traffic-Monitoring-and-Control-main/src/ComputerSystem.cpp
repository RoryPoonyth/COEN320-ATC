#include "ComputerSystem.h"
#include "CommunicationSystem.h"
#include "Radar.h"
#include "Aircraft.h"
#include "ctimer.h"
#include <sys/dispatch.h>
#include <vector>
#include <string>

#define ATTACH_POINT "default"

using namespace std;

ComputerSystem::ComputerSystem() {}

void ComputerSystem::setAircrafts(std::vector<Aircraft*> aircrafts) {
    this->aircrafts = aircrafts;
}

Aircraft* ComputerSystem::getAircraftByFlightId(int flightId) {
    for (Aircraft* aircraft : aircrafts) {
        if (aircraft->getFlightId() == flightId) {
            return aircraft;
        }
    }
    return nullptr;
}

void* ComputerSystem::MapDisplayThread(void* args) {
    ComputerSystem* computerSystem = (ComputerSystem*)args;
    computerSystem->MapDisplay();
    return NULL;
}

void* ComputerSystem::computerSystemThread(void* args) {
    ComputerSystem* computerSystem = (ComputerSystem*)args;
    computerSystem->computerSystemOperations();
    return NULL;
}

void* ComputerSystem::separationCheckThread(void* args) {
    ComputerSystem* computerSystem = (ComputerSystem*)args;
    computerSystem->separationCheck();
    return NULL;
}

/**
 * Function to Display 2D Map
 */
void* ComputerSystem::MapDisplay() {
	//TODO
}

/**
 * Computer System Operations based on OperatorConsole requests
 * 1. Update Aircraft Speed (via CommunicationSystem)
 * 2. Return Aircraft Information (via Radar)
 */

void ComputerSystem::computerSystemOperations() {
	//TODO
}

/**
 * Separation Check Function
 */

void ComputerSystem::separationCheck() {
	//TODO
}

/**
 * Alarm Function
 */
void ComputerSystem::alarm() {
	//TODO
}

ComputerSystem::~ComputerSystem() {}
