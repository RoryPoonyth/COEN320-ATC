#include "OperatorConsole.h"
#include "ComputerSystem.h"
#include "CommunicationSystem.h"
#include "Aircraft.h"

#include <sys/dispatch.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <string>

constexpr char ATTACH_POINT[] = "default";

OperatorConsole::OperatorConsole() {}

void OperatorConsole::writeLog(const std::string& log_entry) {
    std::ofstream logFile("/data/home/qnxuser/OperatorLog.txt", std::ios::app);
    if (!logFile) {
        std::cerr << "Operator commands could not be logged. Error opening log file." << std::endl;
    } else {
        logFile << log_entry << std::endl;
    }
}

void* OperatorConsole::listenForOperatorInput(void* args) {
    auto* operatorConsole = static_cast<OperatorConsole*>(args);
    operatorConsole->listenForUserInput();
    return nullptr;
}

void OperatorConsole::listenForUserInput() {
    while (true) {
        int commandType, flightId;
        std::string log_entry;

        std::cout << "\nEnter your choice (1: Display aircraft info, 2: Update speed, 3: Separation constraint): ";
        std::cin >> commandType;

        switch (commandType) {
            case 1:
                std::cout << "\nEnter Flight Id: ";
                std::cin >> flightId;
                log_entry = "Display flight info for flight #" + std::to_string(flightId);
                writeLog(log_entry);
                displayAircraftInfo(flightId);
                break;

            case 2: {
                std::cout << "\nEnter Flight Id: ";
                std::cin >> flightId;

                int newSpeedX, newSpeedY, newSpeedZ;
                std::cout << "\nEnter SpeedX: ";
                std::cin >> newSpeedX;
                std::cout << "Enter SpeedY: ";
                std::cin >> newSpeedY;
                std::cout << "Enter SpeedZ: ";
                std::cin >> newSpeedZ;

                log_entry = "Change flight speed for flight #" + std::to_string(flightId) +
                            "\nNew speeds (X,Y,Z) = (" + std::to_string(newSpeedX) + "," +
                            std::to_string(newSpeedY) + "," + std::to_string(newSpeedZ) + ")";
                writeLog(log_entry);

                updateAircraftSpeed(flightId, newSpeedX, newSpeedY, newSpeedZ);
                break;
            }

            case 3: {
                int n, p;
                std::cout << "\nEnter N seconds for separation constraint: ";
                std::cin >> n;
                std::cout << "Enter interval for separation constraint: ";
                std::cin >> p;

                log_entry = "Change separation constraint: New input N and P = " + std::to_string(n) + ", " + std::to_string(p);
                writeLog(log_entry);

                setSeparationConstraint(n, p);
                break;
            }

            default:
                std::cerr << "Invalid command type." << std::endl;
                break;
        }
    }
}

void OperatorConsole::displayAircraftInfo(int flightId) {
    std::string attachPoint = std::string(ATTACH_POINT) + "_info";
    AircraftData aircraftCommand = {};
    aircraftCommand.header.type = 0x04;
    aircraftCommand.header.subtype = 0x02;
    aircraftCommand.flightId = flightId;

    int coid = name_open(attachPoint.c_str(), 0);
    if (coid == -1) {
        std::cerr << "Error attaching to channel in displayAircraftInfo: " << strerror(errno) << std::endl;
        return;
    }

    if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), nullptr, 0) == -1) {
        std::cerr << "Error sending message in displayAircraftInfo: " << strerror(errno) << std::endl;
        name_close(coid);
        return;
    }

    name_close(coid);
}

void OperatorConsole::updateAircraftSpeed(int flightId, int newSpeedX, int newSpeedY, int newSpeedZ) {
    std::string attachPoint = std::string(ATTACH_POINT) + "_inner_transfer";
    AircraftData aircraftCommand = {};
    aircraftCommand.header.type = 0x04;
    aircraftCommand.header.subtype = 0x01;
    aircraftCommand.flightId = flightId;
    aircraftCommand.speedX = newSpeedX;
    aircraftCommand.speedY = newSpeedY;
    aircraftCommand.speedZ = newSpeedZ;

    int coid = name_open(attachPoint.c_str(), 0);
    if (coid == -1) {
        std::cerr << "Error attaching to channel in updateAircraftSpeed: " << strerror(errno) << std::endl;
        return;
    }

    if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), nullptr, 0) == -1) {
        std::cerr << "Error sending message in updateAircraftSpeed: " << strerror(errno) << std::endl;
        name_close(coid);
        return;
    }

    name_close(coid);
}

void OperatorConsole::setSeparationConstraint(int n, int p) {
    std::string attachPoint = std::string(ATTACH_POINT) + "_separation";
    SeparationData separationCommand = {};
    separationCommand.n_seconds = n;
    separationCommand.p_interval = p;

    int coid = name_open(attachPoint.c_str(), 0);
    if (coid == -1) {
        std::cerr << "Error attaching to channel in setSeparationConstraint: " << strerror(errno) << std::endl;
        return;
    }

    if (MsgSend(coid, &separationCommand, sizeof(separationCommand), nullptr, 0) == -1) {
        std::cerr << "Error sending message in setSeparationConstraint: " << strerror(errno) << std::endl;
        name_close(coid);
        return;
    }

    name_close(coid);
}

OperatorConsole::~OperatorConsole() {}
