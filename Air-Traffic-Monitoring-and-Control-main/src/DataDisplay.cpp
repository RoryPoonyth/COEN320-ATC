#include "DataDisplay.h"
#include "Aircraft.h"

#include <string>
#include <vector>
#include <iostream>
#include <sys/dispatch.h>

using namespace std;

#define ATTACH_POINT "default"

DataDisplay::DataDisplay() {}

void* DataDisplay::dataDisplayInitializer(void* args) {
    DataDisplay* dataDisplay = static_cast<DataDisplay*>(args);
    dataDisplay->listen();
    return nullptr;
}

void* DataDisplay::MapInitializer(void* args) {
    DataDisplay* dataDisplay = static_cast<DataDisplay*>(args);
    dataDisplay->listenForAircraftMap();
    return nullptr;
}

/**
 * Function used to display the Map of Aircrafts
 */
void* DataDisplay::listenForAircraftMap() {
    string airspace[rows][columns];
    initMap(airspace);

    string attachPoint = string(ATTACH_POINT) + "_MAP";

    name_attach_t* attach;

    if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
        perror("DataDisplay (listenForAircraftMap): Error occurred while creating the attach point");
    }

    vector<AircraftData> aircrafts;
    AircraftData aircraftCommand;
    aircrafts.push_back(aircraftCommand);
    aircrafts[0].header.type = 0x05;
    aircrafts[0].header.subtype = 0x05;

    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircrafts, sizeof(aircrafts), NULL);

        if (rcvid == -1) { // Error condition, exit
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

        if (aircrafts[0].header.type == 0x05 && aircrafts[0].header.subtype == 0x05) {
            vector<Aircraft*> receivedAircrafts;

            for (AircraftData aircraftCommand : aircrafts) {
                Aircraft* aircraft = new Aircraft();
                aircraft->setFlightId(aircraftCommand.flightId);
                aircraft->setPositionX(aircraftCommand.positionX);
                aircraft->setPositionY(aircraftCommand.positionY);
                aircraft->setPositionZ(aircraftCommand.positionZ);
                aircraft->setSpeedX(aircraftCommand.speedX);
                aircraft->setSpeedY(aircraftCommand.speedY);
                aircraft->setSpeedZ(aircraftCommand.speedZ);

                receivedAircrafts.push_back(aircraft);
            }

            updateMap(receivedAircrafts, airspace);

            MsgReply(rcvid, EOK, NULL, 0);
            receivedAircrafts.clear();
            continue;
        } else {
            MsgError(rcvid, ENOSYS);
            continue;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * Function used to listen to display a specific aircraft's info (requested by Operator)
 */
void* DataDisplay::listen() {
    string attachPoint = string(ATTACH_POINT) + "_datadisplay_";

    name_attach_t* attach;

    if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
        perror("DataDisplay (listen): Error occurred while creating the attach point");
    }

    AircraftData aircraftCommand;

    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

        if (rcvid == -1) { // Error condition, exit
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

        if (aircraftCommand.header.type == 0x05 && aircraftCommand.header.subtype == 0x01) {
            cout << endl << endl << "-----------------------------------------------------------------------------" << endl << endl
                 << "(Operator Request) Received aircraft data: flightId = " << aircraftCommand.flightId
                 << ", PositionX = " << aircraftCommand.positionX
                 << ", PositionY = " << aircraftCommand.positionY
                 << ", PositionZ = " << aircraftCommand.positionZ
                 << ", SpeedX = " << aircraftCommand.speedX
                 << ", SpeedY = " << aircraftCommand.speedY
                 << ", SpeedZ = " << aircraftCommand.speedZ
                 << endl << "-----------------------------------------------------------------------------" << endl << endl;

            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        } else {
            MsgError(rcvid, ENOSYS);
            continue;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * -------------------------------------------------------------------------------------------------
 * MAP FUNCTIONS
 * -------------------------------------------------------------------------------------------------
 */

/**
 * Initialize the map
 */
void DataDisplay::initMap(string (&airspace)[ROWS][COLUMNS]) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            airspace[i][j] = " [  ] ";
        }
    }
}

/**
 * Clear previous
 */
void DataDisplay::clearPrevious(string (&airspace)[rows][columns], int indexI, int indexJ, string flightId) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            if (airspace[i][j] == flightId) {
                airspace[i][j] = " [  ] ";
            }
        }
    }
}

/**
 * Write map
 */
void DataDisplay::writeMap(string mapAsString) {
    code = creat("/data/home/qnxuser/MapLog.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (code == -1) {
        cout << "Map could not be logged, error" << endl;
    } else {
        char* blockBuffer = new char[mapAsString.length() + 1];
        sprintf(blockBuffer, "%s", mapAsString.c_str());
        write(code, blockBuffer, mapAsString.length() + 1);
        write(code, "====", 4);
        write(code, "\n", 1);
        delete[] blockBuffer;
        close(code);
    }
}

/**
 * Update map based on current aircraft positions
 */
string DataDisplay::updateMap(vector<Aircraft*>& planes, string (&airspace)[rows][columns]) {
    cout << endl << endl;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            for (size_t k = 0; k < planes.size(); k++) {
                int xCurrent = planes[k]->getPositionX();
                int yCurrent = planes[k]->getPositionY();
                xCurrent /= 1000;
                yCurrent /= 1000;
                try {
                    if (i == yCurrent && j == xCurrent && airspace[i][j] == " [  ] ") {
                        clearPrevious(airspace, i, j, " [ " + to_string(planes[k]->getFlightId()) + " ] ");
                        airspace[i][j] = " [ " + to_string(planes[k]->getFlightId()) + " ] ";
                    }
                } catch (...) {
                    // Most likely out of bounds exception, continue
                }
            }
        }
    }

    string mapAsString = "";
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            cout << airspace[i][j];
            mapAsString += airspace[i][j];
        }
        cout << endl;
        mapAsString += "\n";
    }
    writeMap(mapAsString);
    return mapAsString;
}

DataDisplay::~DataDisplay() {}
