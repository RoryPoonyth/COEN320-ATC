#include "Aircraft.h"
#include "cTimer.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <sys/dispatch.h>

using namespace std;

constexpr char ATTACH_POINT[] = "default";

Aircraft::Aircraft()
    : flightId(0), positionX(0), positionY(0), positionZ(0), speedX(0), speedY(0), speedZ(0), time(0) {}

Aircraft::Aircraft(int flightId, int positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time)
    : flightId(flightId), positionX(positionX), positionY(positionY), positionZ(positionZ),
      speedX(speedX), speedY(speedY), speedZ(speedZ), time(time) {
    initializeAttributes();
}

void Aircraft::initializeAttributes() {
    int err_no = pthread_attr_init(&attr);
    if (err_no != 0) {
        cerr << "ERROR initializing thread attributes: " << err_no << endl;
    }
    err_no = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (err_no != 0) {
        cerr << "ERROR setting detach state for thread attributes: " << err_no << endl;
    }
    err_no = pthread_attr_setstacksize(&attr, 256);
    if (err_no != 0) {
        cerr << "ERROR setting stack size for thread attributes: " << err_no << endl;
    }
}

Aircraft::~Aircraft() {
    if (thread.joinable()) {
        thread.join();
    }
}

int Aircraft::getFlightId() const { return flightId; }
int Aircraft::getPositionX() const { return positionX; }
int Aircraft::getPositionY() const { return positionY; }
int Aircraft::getPositionZ() const { return positionZ; }
int Aircraft::getPrevPositionX() const { return positionX - speedX; }
int Aircraft::getPrevPositionY() const { return positionY - speedY; }
int Aircraft::getPrevPositionZ() const { return positionZ - speedZ; }
int Aircraft::getSpeedX() const { return speedX; }
int Aircraft::getSpeedY() const { return speedY; }
int Aircraft::getSpeedZ() const { return speedZ; }

void Aircraft::setFlightId(int id) { flightId = id; }
void Aircraft::setPositionX(int x) { positionX = x; }
void Aircraft::setPositionY(int y) { positionY = y; }
void Aircraft::setPositionZ(int z) { positionZ = z; }
void Aircraft::setSpeedX(int sx) { speedX = sx; }
void Aircraft::setSpeedY(int sy) { speedY = sy; }
void Aircraft::setSpeedZ(int sz) { speedZ = sz; }

void Aircraft::startThread() {
    thread = std::thread(&Aircraft::planeOperations, this);
    if (!thread.joinable()) {
        cerr << "ERROR starting aircraft thread." << endl;
    }
}

void Aircraft::stopThread() {
    if (thread.joinable()) {
        thread.detach();
    }
}

void Aircraft::planeOperations() {
    cTimer timer(1, 0);  // 1-second interval
    while (true) {
        positionX += speedX;
        positionY += speedY;
        positionZ += speedZ;
        timer.waitTimer();
    }
}

void* Aircraft::listen(const string& attachPoint) {
    auto* attach = name_attach(NULL, attachPoint.c_str(), 0);
    if (!attach) {
        cerr << "Aircraft (listen): Error occurred while creating the attach point." << endl;
        return nullptr;
    }

    AircraftData aircraftCommand;
    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);
        if (rcvid == -1) {
            break;  // Error condition, exit loop
        }

        if (rcvid == 0) {
            handlePulse(aircraftCommand);
            continue;
        }

        processCommand(rcvid, aircraftCommand);
    }

    name_detach(attach, 0);
    return nullptr;
}

void Aircraft::handlePulse(const AircraftData& aircraftCommand) {
    switch (aircraftCommand.header.code) {
        case _PULSE_CODE_DISCONNECT:
            ConnectDetach(aircraftCommand.header.scoid);
            break;
        default:
            break;
    }
}

void Aircraft::processCommand(int rcvid, AircraftData& aircraftCommand) {
    if (aircraftCommand.header.type == 0x00) {
        if (aircraftCommand.header.subtype == 0x01) {
            updateAircraftData(aircraftCommand);
            MsgReply(rcvid, EOK, &aircraftCommand, sizeof(aircraftCommand));
        } else if (aircraftCommand.header.subtype == 0x02) {
            updateSpeed(aircraftCommand);
            MsgReply(rcvid, EOK, NULL, 0);
        } else {
            MsgError(rcvid, ENOSYS);
        }
    } else if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX) {
        MsgError(rcvid, ENOSYS);
    } else {
        MsgReply(rcvid, EOK, NULL, 0);
    }
}

void Aircraft::updateAircraftData(AircraftData& data) const {
    data.flightId = flightId;
    data.positionX = positionX;
    data.positionY = positionY;
    data.positionZ = positionZ;
    data.speedX = speedX;
    data.speedY = speedY;
    data.speedZ = speedZ;
}

void Aircraft::updateSpeed(const AircraftData& data) {
    speedX = data.speedX;
    speedY = data.speedY;
    speedZ = data.speedZ;
}
