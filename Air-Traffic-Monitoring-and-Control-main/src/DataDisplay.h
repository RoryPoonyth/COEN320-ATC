#pragma once

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
    void clearPrevious(string (&airspace)[ROWS][COLUMNS], int indexI, int indexJ, const string& flightId);
    string updateMap(vector<Aircraft*>& planes, string (&airspace)[ROWS][COLUMNS]);
    void writeMap(const string& mapAsString);

    virtual ~DataDisplay();
};
