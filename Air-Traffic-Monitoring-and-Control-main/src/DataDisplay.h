#ifndef SRC_DATADISPLAY_H_
#define SRC_DATADISPLAY_H_

#define ROWS 25
#define COLUMNS 25

#include "Aircraft.h"
#include <string>
#include <vector>

using namespace std;

class DataDisplay {
public:
    DataDisplay();

    int code;

    static void* MapInitializer(void* args);
    static void* dataDisplayInitializer(void* args);

    void* listenForAircraftMap();
    void* listen();
    void initMap(string (&airspace)[ROWS][COLUMNS]);
    void clearPrevious(string (&airspace)[ROWS][COLUMNS], int indexI, int indexJ, string flightId);
    string updateMap(vector<Aircraft*>& planes, string (&airspace)[ROWS][COLUMNS]);
    void writeMap(string mapAsString);

    virtual ~DataDisplay();
};

#endif /* SRC_DATADISPLAY_H_ */
