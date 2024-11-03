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
   //TODO
}

/**
 * Function used to listen to display a specific aircraft's info (requested by Operator)
 */
void* DataDisplay::listen() {
   //TODO
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
