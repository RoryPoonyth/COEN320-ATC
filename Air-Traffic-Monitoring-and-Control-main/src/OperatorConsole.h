#pragma once

#include <string>
#include <iostream>
#include <sys/dispatch.h>

class OperatorConsole {
private:
    std::string log_entry;

public:
    OperatorConsole();

    // Thread functions
    void listenForUserInput();
    void updateAircraftSpeed();
    void displayAircraftData();

    // Log function
    void writeLog(const std::string& log_entry);

    ~OperatorConsole();
};
