#include "Aircraft.h"
#include <iostream>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <cstring>

Aircraft::Aircraft() {}

Aircraft::Aircraft(int flightId, int positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time)
    : flightId(flightId), positionX(positionX), positionY(positionY), positionZ(positionZ),
      speedX(speedX), speedY(speedY), speedZ(speedZ), time(time) {}

Aircraft::~Aircraft() {
    stopThread();
}

int Aircraft::getFlightId() const { return flightId; }
void Aircraft::setFlightId(int id) { flightId = id; }

int Aircraft::getPositionX() const { return positionX; }
int Aircraft::getPositionY() const { return positionY; }
int Aircraft::getPositionZ() const { return positionZ; }
int Aircraft::getPrevPositionX() const { return positionX - speedX; }
int Aircraft::getPrevPositionY() const { return positionY - speedY; }
int Aircraft::getPrevPositionZ() const { return positionZ - speedZ; }
void Aircraft::setPositionX(int x) { positionX = x; }
void Aircraft::setPositionY(int y) { positionY = y; }
void Aircraft::setPositionZ(int z) { positionZ = z; }

int Aircraft::getSpeedX() const { return speedX; }
int Aircraft::getSpeedY() const { return speedY; }
int Aircraft::getSpeedZ() const { return speedZ; }
void Aircraft::setSpeedX(int sx) { speedX = sx; }
void Aircraft::setSpeedY(int sy) { speedY = sy; }
void Aircraft::setSpeedZ(int sz) { speedZ = sz; }

void Aircraft::startThread() {
    if (isRunning) {
        std::cerr << "Thread already running." << std::endl;
        return;
    }
    isRunning = true;
    thread = std::thread(&Aircraft::planeOperations, this);
}

void Aircraft::stopThread() {
    isRunning = false;
    if (thread.joinable()) {
        thread.join();
    }
}

void Aircraft::planeOperations() {
    while (isRunning) {
        move();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Aircraft::move() {
    positionX += speedX;
    positionY += speedY;
    positionZ += speedZ;
}

void Aircraft::listen(const std::string& attachPoint) {
    name_attach_t* attach = name_attach(nullptr, attachPoint.c_str(), 0);
    if (!attach) {
        std::cerr << "Error creating the attach point: " << strerror(errno) << std::endl;
        return;
    }

    AircraftData aircraftCommand;
    while (isRunning) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), nullptr);
        if (rcvid == -1) {
            std::cerr << "Error receiving message: " << strerror(errno) << std::endl;
            break;
        }

        if (rcvid == 0) {
            // Handle pulse
            switch (aircraftCommand.header.code) {
                case _PULSE_CODE_DISCONNECT:
                    ConnectDetach(aircraftCommand.header.scoid);
                    break;
                default:
                    break;
            }
            continue;
        }

        // Process command
        if (aircraftCommand.header.type == 0x00) {
            switch (aircraftCommand.header.subtype) {
                case 0x01:
                    // Respond with current aircraft data
                    aircraftCommand.flightId = flightId;
                    aircraftCommand.positionX = positionX;
                    aircraftCommand.positionY = positionY;
                    aircraftCommand.positionZ = positionZ;
                    aircraftCommand.speedX = speedX;
                    aircraftCommand.speedY = speedY;
                    aircraftCommand.speedZ = speedZ;
                    MsgReply(rcvid, EOK, &aircraftCommand, sizeof(aircraftCommand));
                    break;

                case 0x02:
                    // Update speed with new values
                    setSpeedX(aircraftCommand.speedX);
                    setSpeedY(aircraftCommand.speedY);
                    setSpeedZ(aircraftCommand.speedZ);
                    MsgReply(rcvid, EOK, nullptr, 0);
                    break;

                default:
                    MsgError(rcvid, ENOSYS);
                    break;
            }
        } else {
            MsgError(rcvid, ENOSYS);
        }
    }

    // Clean up the attach point
    if (name_detach(attach, 0) == -1) {
        std::cerr << "Error detaching name attach point: " << strerror(errno) << std::endl;
    }
}

void* Aircraft::aircraftListenHelper(void* args) {
    AircraftListenArgs* listenArgs = static_cast<AircraftListenArgs*>(args);
    listenArgs->aircraft->listen(listenArgs->attachPoint);
    delete listenArgs;
    return nullptr;
}
