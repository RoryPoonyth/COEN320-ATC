#pragma once

#include <string>
#include <iostream>
#include <sys/dispatch.h>

class OperatorConsole {
public:
    OperatorConsole();

    // Thread function for listening to operator input
    static void* listenForOperatorInput(void* args);  // Added declaration for threading

    // Member function to handle user input (used by the thread)
    void listenForUserInput();

    // Other command functions
    void displayAircraftInfo(int flightId);
    void updateAircraftSpeed(int flightId, int newSpeedX, int newSpeedY, int newSpeedZ);
    void setSeparationConstraint(int n, int p);

    // Log function
    void writeLog(const std::string& log_entry);

    ~OperatorConsole();
};
