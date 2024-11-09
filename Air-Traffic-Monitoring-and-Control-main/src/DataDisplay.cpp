#include "DataDisplay.h"
#include "Aircraft.h"

#include <string>
#include <vector>
#include <iostream>
#include <sys/dispatch.h>
#include <iomanip>
#include <sstream>
#include <fcntl.h> // For file operations
#include <unistd.h> // For close()
#include <stdlib.h> // For EXIT_SUCCESS

using namespace std;

#define ATTACH_POINT "default" // Base attach point for inter-process communication

// Constructor: Initializes DataDisplay with default code value
DataDisplay::DataDisplay() {
    code = -1;
}

// Initializes the listener for operator data display requests
void* DataDisplay::dataDisplayInitializer(void* args) {
    DataDisplay* dataDisplay = (DataDisplay*)args;
    dataDisplay->listen();
    return NULL;
}

// Initializes the listener for aircraft map updates
void* DataDisplay::MapInitializer(void* args) {
    DataDisplay* dataDisplay = (DataDisplay*)args;
    dataDisplay->listenForAircraftMap();
    return NULL;
}

/**
 * Function to continuously listen for updates to the map of aircraft.
 * Updates the internal airspace map based on received aircraft data.
 */
void* DataDisplay::listenForAircraftMap() {
    string airspace[rows][columns];
    initMap(airspace); // Initialize the map to an empty state

    string attachPoint = string(ATTACH_POINT) + "_MAP"; // Attach point for inter-process communication

    name_attach_t *attach;

    // Establish attach point for incoming map data
    if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
        perror("DataDisplay (listenForAircraftMap): Error occurred while creating the attach point");
    }

    vector<AircraftData> aircrafts; // Holds received aircraft data
    AircraftData aircraftCommand; // Temporary storage for incoming commands
    aircrafts.push_back(aircraftCommand);
    aircrafts[0].header.type = 0x05;
    aircrafts[0].header.subtype = 0x05;

    while (true) {
        // Receive messages continuously
        int rcvid = MsgReceive(attach->chid, &aircrafts, sizeof(aircrafts), NULL);

        if (rcvid == -1) { // Error in receiving, exit loop
            break;
        }

        if (rcvid == 0) { // Handle pulse messages
            switch (aircraftCommand.header.code) {
            case _PULSE_CODE_DISCONNECT:
                // Handle client disconnection
                ConnectDetach(aircraftCommand.header.scoid);
                continue;
            case _PULSE_CODE_UNBLOCK:
                // REPLY blocked client wants to unblock
                break;
            default:
                // Ignore other pulses
                break;
            }
            continue;
        }

        // Handle connection messages from name_open()
        if (aircraftCommand.header.type == _IO_CONNECT ) {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        }

        // Reject unhandled QNX IO messages
        if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX ) {
            MsgError(rcvid, ENOSYS);
            continue;
        }

        // If aircraft data type and subtype are correct, update the map
        if (aircrafts[0].header.type == 0x05 && aircrafts[0].header.subtype == 0x05) {
            vector<Aircraft*> receivedAircrafts;

            // Process each incoming aircraft's data
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

            updateMap(receivedAircrafts, airspace); // Update the display with new aircraft positions

            // Acknowledge receipt of message to CommandSystem
            MsgReply(rcvid, EOK, NULL, 0);

            receivedAircrafts.clear(); // Clear data after processing
            continue; // Listen for the next request
        }
    }

    return EXIT_SUCCESS;
}

/**
 * Function to listen and display specific aircraft info based on operator requests.
 */
void* DataDisplay::listen() {
    string attachPoint = string(ATTACH_POINT) + "_datadisplay_"; // Attach point for specific data display requests

    name_attach_t *attach;

    // Establish the attach point for incoming data display requests
    if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
        perror("DataDisplay (listen): Error occurred while creating the attach point");
    }

    AircraftData aircraftCommand; // Holds data received from operator requests

    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

        if (rcvid == -1) { // Error in receiving, exit loop
            break;
        }

        if (rcvid == 0) { // Handle pulse messages
            switch (aircraftCommand.header.code) {
            case _PULSE_CODE_DISCONNECT:
                // Handle client disconnection
                ConnectDetach(aircraftCommand.header.scoid);
                continue;
            case _PULSE_CODE_UNBLOCK:
                // REPLY blocked client wants to unblock
                break;
            default:
                // Ignore other pulses
                break;
            }
            continue;
        }

        // Handle connection messages from name_open()
        if (aircraftCommand.header.type == _IO_CONNECT) {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        }

        // Reject unhandled QNX IO messages
        if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX) {
            MsgError(rcvid, ENOSYS);
            continue;
        }

        // Process operator requests to display specific aircraft information
        if (aircraftCommand.header.type == 0x05 && aircraftCommand.header.subtype == 0x01) {
            printAircraftData(aircraftCommand);

            // Acknowledge receipt of message
            MsgReply(rcvid, EOK, NULL, 0);
            continue; // Listen for the next request
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
 * Initialize the airspace map with empty cells.
 */
void DataDisplay::initMap(string (&airspace)[rows][columns]) {
    for(int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            airspace[i][j] = formatCellContent(" -- ");
        }
    }
}

/**
 * Clear the previous aircraft positions from the map.
 */
void DataDisplay::clearPrevious(string (&airspace)[rows][columns], int flightId) {
    string flightIdStr = formatCellContent("F" + to_string(flightId));
    for(int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            if(airspace[i][j] == flightIdStr) {
                airspace[i][j] = formatCellContent(" -- ");
            }
        }
    }
}

/**
 * Write the current map state to a log file.
 */
void DataDisplay::writeMap(string mapAsString) {
    code = creat("/data/home/qnxuser/MapLog.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if(code == -1) {
        cout << "Map could not be logged, error" << endl;
    } else {
        // Buffer the map data to write to the log file
        char *blockBuffer = new char[mapAsString.length() + 1];
        sprintf(blockBuffer, "%s", mapAsString.c_str());
        write(code, blockBuffer, mapAsString.length() + 1);
        write(code, "====", 4);
        write(code, "\n", 1);
        delete[] blockBuffer;
        close(code);
    }
}

/**
 * Format the cell content to have consistent width.
 */
string DataDisplay::formatCellContent(const string& content) {
    if (content.length() >= 6) {
        return content.substr(0, 6);
    } else {
        return content + string(6 - content.length(), ' ');
    }
}

/**
 * Print the map with a timestamp and separator.
 */
void DataDisplay::printMapWithTimestamp(string (&airspace)[rows][columns]) {
    // Get the current time
    time_t now = time(0);
    char* dt = ctime(&now);

    // Print separator and timestamp
    cout << endl;
    cout << "=============================================================================" << endl;
    cout << "Map Update at " << dt;
    cout << "=============================================================================" << endl;

    // Print x-axis labels
    cout << "     "; // Offset for y-axis labels
    for (int j = 0; j < columns; j++) {
        cout << setw(6) << j * 1000;
    }
    cout << endl;

    // Print map with y-axis labels
    for (int i = 0; i < rows; i++) {
        cout << setw(5) << i * 1000; // y-axis label
        for (int j = 0; j < columns; j++) {
            cout << airspace[i][j];
        }
        cout << endl;
    }
    cout << endl;
}

/**
 * Update the map based on current aircraft positions.
 */
string DataDisplay::updateMap(vector<Aircraft*>& planes, string (&airspace)[rows][columns]) {
    // Each cell represents a 1000 ft area

    // Clear the previous positions of all aircraft
    initMap(airspace);

    for(size_t k = 0; k < planes.size(); k++) {
        int xCurrent = planes[k]->getPositionX();
        int yCurrent = planes[k]->getPositionY();

        // Fit aircraft in 1000 sq. ft cells
        xCurrent /= 1000;
        yCurrent /= 1000;

        // Ensure indices are within bounds
        if (xCurrent >= 0 && xCurrent < columns && yCurrent >= 0 && yCurrent < rows) {
            string flightIdStr = "F" + to_string(planes[k]->getFlightId());
            airspace[yCurrent][xCurrent] = formatCellContent(flightIdStr);
        }
    }

    // Print the map with a timestamp
    printMapWithTimestamp(airspace);

    // Convert the map to a string for logging
    stringstream mapAsStringStream;
    // First, add x-axis labels
    mapAsStringStream << "     ";
    for (int j = 0; j < columns; j++) {
        mapAsStringStream << setw(6) << j * 1000;
    }
    mapAsStringStream << endl;
    // Add the map content
    for (int i = 0; i < rows; i++) {
        mapAsStringStream << setw(5) << i * 1000;
        for (int j = 0; j < columns; j++) {
            mapAsStringStream << airspace[i][j];
        }
        mapAsStringStream << endl;
    }
    string mapAsString = mapAsStringStream.str();
    writeMap(mapAsString);
    return mapAsString;
}

/**
 * Display aircraft data in a formatted table.
 */
void DataDisplay::printAircraftData(const AircraftData& data) {
    cout << endl << "-----------------------------------------------------------------------------" << endl;
    cout << setw(15) << "Flight ID" << setw(15) << "Pos X" << setw(15) << "Pos Y" << setw(15) << "Pos Z"
         << setw(15) << "Speed X" << setw(15) << "Speed Y" << setw(15) << "Speed Z" << endl;
    cout << setw(15) << data.flightId
         << setw(15) << data.positionX
         << setw(15) << data.positionY
         << setw(15) << data.positionZ
         << setw(15) << data.speedX
         << setw(15) << data.speedY
         << setw(15) << data.speedZ << endl;
    cout << "-----------------------------------------------------------------------------" << endl;
}

// Destructor to clean up resources, if necessary
DataDisplay::~DataDisplay() {
}
