#include "Radar.h"
#include "Aircraft.h"

#include <iostream>
#include <vector>
#include <string>
#include <sys/dispatch.h>
#include <cerrno>
#include <cstring>

constexpr char ATTACH_POINT[] = "default";

Radar::Radar() {}

Radar::Radar(const std::vector<Aircraft>& aircrafts) : aircrafts(aircrafts) {}

Radar::Radar(const Aircraft& aircraft) : aircraft(aircraft) {}

void* Radar::radarInitializer(void* args) {
    auto* radar = static_cast<Radar*>(args);
    radar->pingAircraft();
    return nullptr;
}

void* Radar::pingAircraft() {
    std::string attachPoint = std::string(ATTACH_POINT) + "_RADAR";
    name_attach_t* attach = name_attach(nullptr, attachPoint.c_str(), 0);
    if (!attach) {
        std::cerr << "Radar (pingAircraft): Error occurred while creating the attach point: " << strerror(errno) << std::endl;
        return (void*)EXIT_FAILURE;
    }

    AircraftData aircraftCommand;
    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), nullptr);
        if (rcvid == -1) {
            break;
        }

        if (rcvid == 0) { // Pulse received
            handlePulse(aircraftCommand);
            continue;
        }

        if (aircraftCommand.header.type == _IO_CONNECT) {
            MsgReply(rcvid, EOK, nullptr, 0);
            continue;
        }

        if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX) {
            MsgError(rcvid, ENOSYS);
            continue;
        }

        if (aircraftCommand.header.type == 0x08 && aircraftCommand.header.subtype == 0x08) {
            handleAircraftRequest(rcvid, aircraftCommand);
        } else {
            MsgError(rcvid, ENOSYS);
        }
    }

    name_detach(attach, 0);
    return EXIT_SUCCESS;
}

void Radar::handlePulse(const AircraftData& aircraftCommand) const {
    switch (aircraftCommand.header.code) {
        case _PULSE_CODE_DISCONNECT:
            ConnectDetach(aircraftCommand.header.scoid);
            break;
        default:
            break;
    }
}

void Radar::handleAircraftRequest(int rcvid, AircraftData& aircraftCommand) const {
    std::string attachPoint = std::string(ATTACH_POINT) + "_" + std::to_string(aircraftCommand.flightId);
    int server_coid = name_open(attachPoint.c_str(), 0);
    if (server_coid == -1) {
        std::cerr << "Error occurred while attaching to channel: " << strerror(errno) << std::endl;
        MsgError(rcvid, ENOSYS);
        return;
    }

    aircraftCommand.header.type = 0x00;
    aircraftCommand.header.subtype = 0x01;

    if (MsgSend(server_coid, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
        std::cerr << "Error while sending message to aircraft: " << strerror(errno) << std::endl;
        name_close(server_coid);
    } else {
        MsgReply(rcvid, EOK, &aircraftCommand, sizeof(aircraftCommand));
    }

    name_close(server_coid);
}

AircraftData Radar::operatorRequestPingAircraft(int flightId) const {
    std::string attachPoint = std::string(ATTACH_POINT) + "_" + std::to_string(flightId);
    int server_coid = name_open(attachPoint.c_str(), 0);
    if (server_coid == -1) {
        std::cerr << "Error occurred while attaching to channel: " << strerror(errno) << std::endl;
        return {};
    }

    AircraftData aircraftCommand{};
    aircraftCommand.header.type = 0x00;
    aircraftCommand.header.subtype = 0x01;

    if (MsgSend(server_coid, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
        std::cerr << "Error while sending message to aircraft: " << strerror(errno) << std::endl;
    }

    name_close(server_coid);
    return aircraftCommand;
}

Radar::~Radar() {}
