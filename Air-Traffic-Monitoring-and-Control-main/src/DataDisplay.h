#ifndef SRC_DATADISPLAY_H_
#define SRC_DATADISPLAY_H_

#include "Aircraft.h"
#include <string>
#include <vector>
#include <mutex>

using namespace std;

// Constants for map dimensions
const int rows = 25;
const int columns = 25;

// DataDisplay class: Manages the display and tracking of aircraft positions on a map
class DataDisplay {
public:
    DataDisplay(); // Constructor to initialize the DataDisplay object
    int code; // Code used for logging or status purposes

    // Static initializer function for creating a thread to display the aircraft map
    static void* MapInitializer(void* args);

    // Static initializer function for creating a thread to listen to aircraft data display requests
    static void* dataDisplayInitializer(void* args);

    // Listens for incoming aircraft map data and displays it on the map
    void* listenForAircraftMap();

    // Listens for specific aircraft information requests from the operator console
    void* listen();

    // Initializes the airspace map by setting all cells to empty
    void initMap(string (&airspace)[rows][columns]);

    // Clears the previous positions of a specified aircraft on the map
    void clearPrevious(string (&airspace)[rows][columns], int flightId);

    // Updates the map with the current positions of all aircraft and returns the map as a string
    string updateMap(vector<Aircraft*>& planes, string (&airspace)[rows][columns]);

    // Writes the map state to a log file for record-keeping or debugging purposes
    void writeMap(string mapAsString);

    // Formats cell content for consistent display
    string formatCellContent(const string& content);

    // Prints the map with a timestamp
    void printMapWithTimestamp(string (&airspace)[rows][columns]);

    // Displays aircraft data in a formatted table
    void printAircraftData(const AircraftData& data);

    virtual ~DataDisplay(); // Destructor to clean up resources used by DataDisplay

};

#endif /* SRC_DATADISPLAY_H_ */
