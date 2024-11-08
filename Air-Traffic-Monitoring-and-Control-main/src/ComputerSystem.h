#pragma once

#include "Aircraft.h"
#include <vector>

// Data structure to hold separation constraint details
typedef struct {
    int n_seconds;    // Number of seconds for separation check
    int p_interval;   // Interval for checking separation
} SeparationData;

class ComputerSystem {
public:
    ComputerSystem();

    // Static thread functions
    static void* MapDisplayThread(void* args);
    static void* dataDisplayThread(void* args);
    static void* computerSystemThread(void* args);
    static void* separationCheckThread(void* args);

    // Member functions for Computer System operations
    void* MapDisplay();                          // Displays the map of aircrafts
    void setAircrafts(const std::vector<Aircraft*>& aircrafts);  // Set the list of aircraft pointers
    Aircraft* getAircraftByFlightId(int flightId); // Get an aircraft by its flight ID
    void computerSystemOperations();             // Perform operations related to the computer system
    void separationCheck();                      // Check separation between aircrafts
    void alarm();                                // Trigger an alarm if a safety condition is violated

    virtual ~ComputerSystem();

private:
    std::vector<Aircraft*> aircrafts;            // Vector of aircraft pointers managed by the computer system
};
