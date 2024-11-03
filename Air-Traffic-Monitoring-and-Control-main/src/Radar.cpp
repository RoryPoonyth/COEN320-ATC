#include "Radar.h"
#include "Aircraft.h"

#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <sys/dispatch.h>

using namespace std;

#define ATTACH_POINT "default"

Radar::Radar() {}

Radar::Radar(vector<Aircraft>& aircrafts) {
    this->aircrafts = aircrafts;
}

Radar::Radar(Aircraft& aircraft) {
    this->aircraft = aircraft;
}

void* Radar::radarInitializer(void* args) {
    Radar* radar = (Radar*)args;
    radar->pingAircraft();
    return NULL;
}

void* Radar::pingAircraft() {
    string attachPoint = string(ATTACH_POINT) + "_RADAR";

    name_attach_t* attach;

    if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
        perror("Radar (pingAircraft): Error occurred while creating the attach point");
    }

    AircraftData aircraftCommand;
    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

        if (rcvid == -1) {
            break;
        }

        if (rcvid == 0) { /* Pulse received */
            switch (aircraftCommand.header.code) {
                case _PULSE_CODE_DISCONNECT:
                    ConnectDetach(aircraftCommand.header.scoid);
                    continue;
                case _PULSE_CODE_UNBLOCK:
                    break;
                default:
                    break;
            }
            continue;
        }

        if (aircraftCommand.header.type == _IO_CONNECT) {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        }

        if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX) {
            MsgError(rcvid, ENOSYS);
            continue;
        }

        if (aircraftCommand.header.type == 0x08 && aircraftCommand.header.subtype == 0x08) {
            string attachPoint = string(ATTACH_POINT) + "_" + to_string(aircraftCommand.flightId);

            int server_coid;
            if ((server_coid = name_open(attachPoint.c_str(), 0)) == -1) {
                perror("Error occurred while attaching the channel");
            }

            AircraftData aircraftCommand;
            aircraftCommand.header.type = 0x00;
            aircraftCommand.header.subtype = 0x01;

            if (MsgSend(server_coid, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
                cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
                name_close(server_coid);
            }

            MsgReply(rcvid, EOK, &aircraftCommand, sizeof(aircraftCommand));
            continue;
        } else {
            MsgError(rcvid, ENOSYS);
            continue;
        }
    }

    return EXIT_SUCCESS;
}

AircraftData Radar::operatorRequestPingAircraft(int flightId) {
    string attachPoint = string(ATTACH_POINT) + "_" + to_string(flightId);

    int server_coid;
    if ((server_coid = name_open(attachPoint.c_str(), 0)) == -1) {
        perror("Error occurred while attaching the channel");
    }

    AircraftData aircraftCommand;
    aircraftCommand.header.type = 0x00;
    aircraftCommand.header.subtype = 0x01;

    if (MsgSend(server_coid, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
        cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
        name_close(server_coid);
    }

    return aircraftCommand;
}

Radar::~Radar() {}
