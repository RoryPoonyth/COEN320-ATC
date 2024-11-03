#include "OperatorConsole.h"
#include "ComputerSystem.h"
#include "CommunicationSystem.h"
#include "Aircraft.h"

#include <sys/dispatch.h>
#include <vector>
#include <string>

#define ATTACH_POINT "default"

using namespace std;

OperatorConsole::OperatorConsole() {}

void OperatorConsole::writeLog(string log_entry) {
    int code = creat("/data/home/qnxuser/OperatorLog.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (code == -1) {
        cout << "Operator commands could not be logged, error" << endl;
    } else {
        char *blockBuffer = new char[log_entry.length() + 1];
        sprintf(blockBuffer, "%s", log_entry.c_str());
        write(code, blockBuffer, log_entry.length() + 1);
        write(code, "\n", 1);
        delete[] blockBuffer;
        close(code);
    }
}

void* OperatorConsole::listenForOperatorInput(void* args) {
    OperatorConsole* operatorConsole = (OperatorConsole*)args;
    operatorConsole->listenForUserInput();
    return NULL;
}

void* OperatorConsole::updateAircraftSpeedThread(void* args) {
    OperatorConsole* operatorConsole = (OperatorConsole*)args;
    operatorConsole->updateAircraftSpeed();
    return NULL;
}

void* OperatorConsole::displayAircraftDataThread(void* args) {
    OperatorConsole* operatorConsole = (OperatorConsole*)args;
    operatorConsole->displayAircraftData();
    return NULL;
}

/**
 * Listen For User Input
 */

void OperatorConsole::listenForUserInput() {
    while (true) {
        int commandType;
        int flightId;

        cout << endl << "Enter your choice... (1 to display an aircraft's info, 2 to update an aircraft's speed, 3 for separation constraint)" << endl << endl;
        cin >> commandType;

        if (commandType == 1) {
            cout << endl << "Enter Flight Id: ";
            cin >> flightId;
            log_entry += "Display flight info for flight #";
            log_entry += to_string(flightId);
            writeLog(log_entry);
            log_entry = "";

            pthread_t displayDataThread;
            pthread_create(&displayDataThread, nullptr, &OperatorConsole::displayAircraftDataThread, this);

            AircraftData aircraftCommand;
            aircraftCommand.header.type = 0x04;
            aircraftCommand.header.subtype = 0x02;
            aircraftCommand.flightId = flightId;

            sleep(1);

            string attachPoint = string(ATTACH_POINT) + "info";

            int coid;
            if ((coid = name_open(attachPoint.c_str(), 0)) == -1) {
                perror("Error occurred while attaching the channel listenForUserInput FUNCTION [1]");
                return;
            }

            if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
                cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
                name_close(coid);
                return;
            }

        } else if (commandType == 2) {
            cout << endl << "Enter Flight Id: ";
            cin >> flightId;
            log_entry += "Change flight speed for flight #";
            log_entry += to_string(flightId);
            log_entry += "\nNew speeds (X,Y,Z) = (";

            pthread_t updateAircraftSpeedThread;
            pthread_create(&updateAircraftSpeedThread, nullptr, &OperatorConsole::updateAircraftSpeedThread, this);

            int newSpeedX, newSpeedY, newSpeedZ;

            cout << endl << "Enter SpeedX: ";
            cin >> newSpeedX;
            log_entry += to_string(newSpeedX) + ",";

            cout << endl << "Enter SpeedY: ";
            cin >> newSpeedY;
            log_entry += to_string(newSpeedY) + ",";

            cout << endl << "Enter SpeedZ: ";
            cin >> newSpeedZ;
            log_entry += to_string(newSpeedZ) + ")";
            writeLog(log_entry);
            log_entry = "";

            AircraftData aircraftCommand;
            aircraftCommand.header.type = 0x04;
            aircraftCommand.header.subtype = 0x01;
            aircraftCommand.flightId = flightId;
            aircraftCommand.speedX = newSpeedX;
            aircraftCommand.speedY = newSpeedY;
            aircraftCommand.speedZ = newSpeedZ;

            string attachPoint = string(ATTACH_POINT) + "inner_transfer";

            int coid;
            if ((coid = name_open(attachPoint.c_str(), 0)) == -1) {
                perror("Error occurred while attaching the channel listenForUserInput FUNCTION [2]");
                return;
            }

            if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
                cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
                name_close(coid);
                return;
            }

        } else if (commandType == 3) {
            log_entry += "Change input N and interval P at runtime for separation constraint \nNew input N and P: ";

            int n;
            cout << endl << "Enter N seconds for separation constraint: ";
            cin >> n;
            log_entry += to_string(n) + ", ";

            int p;
            cout << endl << "Enter interval for separation constraint: ";
            cin >> p;
            log_entry += to_string(p) + "\n";

            string attachPoint = string(ATTACH_POINT) + "_separation";

            int coid;
            if ((coid = name_open(attachPoint.c_str(), 0)) == -1) {
                perror("Error occurred while attaching the channel listenForUserInput FUNCTION [3]");
                return;
            }

            SeparationData separationCommand;
            separationCommand.n_seconds = n;
            separationCommand.p_interval = p;

            if (MsgSend(coid, &separationCommand, sizeof(separationCommand), NULL, 0) == -1) {
                cout << "Error while sending the message for computer system: " << strerror(errno) << endl;
                name_close(coid);
                return;
            }

        } else {
            cout << "Invalid command type." << endl;
        }
    }
}

void* OperatorConsole::updateAircraftSpeed() {
    name_attach_t *attach;

    string attachPointInner = string(ATTACH_POINT) + "inner_transfer";

    if ((attach = name_attach(NULL, attachPointInner.c_str(), 0)) == NULL) {
        perror("Error occurred while creating the attach point updateAircraftSpeed");
        return (void*)EXIT_FAILURE;
    }

    AircraftData aircraftCommand;
    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

        if (rcvid == -1) {
            break;
        }

        if (aircraftCommand.header.type == 0x04 && aircraftCommand.header.subtype == 0x01) {
            MsgReply(rcvid, EOK, NULL, 0);
            name_detach(attach, 0);

            int coid;
            if ((coid = name_open(ATTACH_POINT, 0)) == -1) {
                perror("Error occurred while attaching the channel UPDATEAIRCRAFTSPEED FUNCTION");
                return (void*)EXIT_FAILURE;
            }

            aircraftCommand.header.type = 0x02;
            aircraftCommand.header.subtype = 0x00;

            if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
                cout << "Error while sending the message for aircraft CSOP: " << strerror(errno) << endl;
                name_close(coid);
                return (void*)EXIT_FAILURE;
            }

            break;
        }
    }

    return EXIT_SUCCESS;
}

void* OperatorConsole::displayAircraftData() {
    name_attach_t *attach;

    string attachPointInner = string(ATTACH_POINT) + "info";

    if ((attach = name_attach(NULL, attachPointInner.c_str(), 0)) == NULL) {
        perror("Error occurred while creating the attach point displayAircraftData");
        return (void*)EXIT_FAILURE;
    }

    AircraftData aircraftCommand;
    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

        if (rcvid == -1) {
            break;
        }

        if (aircraftCommand.header.type == 0x04 && aircraftCommand.header.subtype == 0x02) {
            int coid;
            if ((coid = name_open(ATTACH_POINT, 0)) == -1) {
                perror("Error occurred while attaching the channel UPDATEAIRCRAFTSPEED FUNCTION");
                return (void*)EXIT_FAILURE;
            }

            aircraftCommand.header.type = 0x02;
            aircraftCommand.header.subtype = 0x01;

            if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
                cout << "Error while sending the message for aircraft CSOP: " << strerror(errno) << endl;
                name_close(coid);
                return (void*)EXIT_FAILURE;
            }

            MsgReply(rcvid, EOK, NULL, 0);
            name_detach(attach, 0);

            break;
        }
    }

    return EXIT_SUCCESS;
}

OperatorConsole::~OperatorConsole() {}
