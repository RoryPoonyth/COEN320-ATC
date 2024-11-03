#include "ComputerSystem.h"
#include "CommunicationSystem.h"
#include "Radar.h"
#include "Aircraft.h"
#include "ctimer.h"
#include <sys/dispatch.h>
#include <vector>
#include <string>

#define ATTACH_POINT "default"

using namespace std;

ComputerSystem::ComputerSystem() {}

void ComputerSystem::setAircrafts(std::vector<Aircraft*> aircrafts) {
    this->aircrafts = aircrafts;
}

Aircraft* ComputerSystem::getAircraftByFlightId(int flightId) {
    for (Aircraft* aircraft : aircrafts) {
        if (aircraft->getFlightId() == flightId) {
            return aircraft;
        }
    }
    return nullptr;
}

void* ComputerSystem::MapDisplayThread(void* args) {
    ComputerSystem* computerSystem = (ComputerSystem*)args;
    computerSystem->MapDisplay();
    return NULL;
}

void* ComputerSystem::computerSystemThread(void* args) {
    ComputerSystem* computerSystem = (ComputerSystem*)args;
    computerSystem->computerSystemOperations();
    return NULL;
}

void* ComputerSystem::separationCheckThread(void* args) {
    ComputerSystem* computerSystem = (ComputerSystem*)args;
    computerSystem->separationCheck();
    return NULL;
}

/**
 * Function to Display 2D Map
 */
void* ComputerSystem::MapDisplay() {
    // Repeat infinitely with a period of 5 seconds
    while (true) {
        vector<AircraftData> aircraftsInfo;
        Radar radar;

        // Initialize thread vector for thread deletion at the end of the current period
        vector<pthread_t> threads;

        // Create the Aircraft listen threads for each aircraft
        for (size_t i = 0; i < aircrafts.size(); i++) {
            string attachPointRAD = string(ATTACH_POINT) + "_" + to_string(aircrafts[i]->getFlightId());
            pthread_t thread;
            AircraftListenArgs* args = new AircraftListenArgs{aircrafts[i], attachPointRAD};
            pthread_create(&thread, NULL, &Aircraft::aircraftListenHelper, args);

            // Add to vector for thread deletion
            threads.push_back(thread);
        }

        // Store received aircraft info into aircraftsInfo vector
        for (size_t i = 0; i < aircrafts.size(); i++) {
            // TODO: Replace with IPC with Radar
            AircraftData aircraftResult = radar.operatorRequestPingAircraft(aircrafts[i]->getFlightId());
            aircraftsInfo.push_back(aircraftResult);
        }

        // Delete aircraft listen threads created during this period
        for (pthread_t thread : threads) {
            pthread_cancel(thread);
        }

        // Set header type and subtype for communication with DataDisplay
        aircraftsInfo[0].header.type = 0x05;
        aircraftsInfo[0].header.subtype = 0x05;

        string attachPointDataDisplay = string(ATTACH_POINT) + "_MAP";

        // Establish connection (communication) with DataDisplay
        int coid;
        if ((coid = name_open(attachPointDataDisplay.c_str(), 0)) == -1) {
            perror("ComputerSystem (MapDisplay): Error occurred while attaching the channel");
            return (void*)EXIT_FAILURE;
        }

        // Send aircrafts info to DataDisplay for Map, expect no response
        if (MsgSend(coid, &aircraftsInfo, sizeof(aircraftsInfo), NULL, 0) == -1) {
            cout << "Error while sending the message: " << strerror(errno) << endl;
            name_close(coid);
            return (void*)EXIT_FAILURE;
        }

        // Repeat every 5 seconds
        sleep(5);
    }

    return EXIT_SUCCESS;
}

/**
 * Computer System Operations based on OperatorConsole requests
 * 1. Update Aircraft Speed (via CommunicationSystem)
 * 2. Return Aircraft Information (via Radar)
 */
void ComputerSystem::computerSystemOperations() {
    name_attach_t *attach;

    if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
        perror("Error occurred while creating the attach point sendNewAircraftSpeedToCommunicationSystem");
    }

    int newSpeedX, newSpeedY, newSpeedZ;
    AircraftData aircraftCommand;
    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

        if (rcvid == -1) { // Error condition, exit
            break;
        }

        /**
         * Updates Aircraft Speed through the Communication System based on Operator Console
         */
        if (aircraftCommand.header.type == 0x02 && aircraftCommand.header.subtype == 0x00) {
            newSpeedX = aircraftCommand.speedX;
            newSpeedY = aircraftCommand.speedY;
            newSpeedZ = aircraftCommand.speedZ;
            MsgReply(rcvid, EOK, NULL, 0);

            // Retrieve aircraft based on aircraftCommand.flightId (received from Operator)
            Aircraft* aircraft = getAircraftByFlightId(aircraftCommand.flightId);
            if (aircraft == nullptr) {
                cout << "Error: Aircraft with flight ID " << aircraftCommand.flightId << " was not FOUND." << endl;
                continue;
            }

            // Create the Aircraft Listen Thread for the Aircraft Listen Function and then Call CommunicationSystem.RelayNewSpeed
            string attachPoint = string(ATTACH_POINT) + "_" + to_string(aircraftCommand.flightId);
            pthread_t thread;
            AircraftListenArgs* args = new AircraftListenArgs{aircraft, attachPoint};
            pthread_create(&thread, NULL, &Aircraft::aircraftListenHelper, args);
            CommunicationSystem communicationSystem;
            communicationSystem.relayNewSpeed(*aircraft, newSpeedX, newSpeedY, newSpeedZ);
            alarm();
            continue;

        /**
         * Return Aircraft Information (Operator Console Request)
         */
        } else if (aircraftCommand.header.type == 0x02 && aircraftCommand.header.subtype == 0x01) {
            // Reply without data to OperatorConsole
            MsgReply(rcvid, EOK, NULL, 0);

            Aircraft* aircraft = getAircraftByFlightId(aircraftCommand.flightId);
            if (aircraft == nullptr) {
                cout << "Error: Aircraft with flight ID " << aircraftCommand.flightId << " was not found." << endl;
                continue;
            }

            // Create the Aircraft Listen Thread for the Aircraft Listen Function
            string attachPointRAD = string(ATTACH_POINT) + "_" + to_string(aircraftCommand.flightId);
            pthread_t thread;
            AircraftListenArgs* args = new AircraftListenArgs{aircraft, attachPointRAD};
            pthread_create(&thread, NULL, &Aircraft::aircraftListenHelper, args);

            // Create the attach point for radar
            string radarAttachPoint = string(ATTACH_POINT) + "_RADAR";

            int radarCOID;
            if ((radarCOID = name_open(radarAttachPoint.c_str(), 0)) == -1) {
                perror("Computer System (computerSystemOperations): Error occurred while attaching the channel for Radar Request");
                return;
            }

            aircraftCommand.header.type = 0x08;
            aircraftCommand.header.subtype = 0x08;

            // Send data to Radar expecting response with Aircraft Information
            if (MsgSend(radarCOID, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
                cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
                name_close(radarCOID);
            }

            // Send the received data to DataDisplay:

            // Generate the unique attach point name
            string attachPointDATADISPLAY = string(ATTACH_POINT) + "_datadisplay_";

            // Establish connection (communication)
            int coid;
            if ((coid = name_open(attachPointDATADISPLAY.c_str(), 0)) == -1) {
                perror("Error occurred while attaching the channel IN COMPSYS FOR DATADISPLAY");
                return;
            }

            aircraftCommand.header.type = 0x05;
            aircraftCommand.header.subtype = 0x01;

            // Send data to DataDisplay expecting no response
            if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), NULL, 0) == -1) {
                cout << "Error while sending the message for aircraft: " << strerror(errno) << endl;
                name_close(coid);
                return;
            }

            continue;
        }
    }
    // THIS IS NEVER REACHED
    name_detach(attach, 0);
}

/**
 * Separation Check Function
 */
void ComputerSystem::separationCheck() {
    name_attach_t *attach;

    string attachPoint = string(ATTACH_POINT) + "_separation";

    if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
        perror("Error occurred while creating the attach point separation");
    }
    int n = 180;
    int p = 5;
    SeparationData separationCommand;

    cTimer timer(p, 0);
    while (true) {
        alarm();
        int rcvid;
        for (int i = 0; i < 2; i++) { // Stop looking for the message after 200 tries
            if ((rcvid = MsgReceive(attach->chid, &separationCommand, sizeof(separationCommand), NULL)) != -1) {
                n = separationCommand.n_seconds;
                timer.setTimerSpec(separationCommand.p_interval, 0);
                cout << "Message Received N: " << n << " P:" << separationCommand.p_interval << endl;
                MsgReply(rcvid, EOK, NULL, 0);
                i = 200;
            }
        }

        string output = "";
        int width = 3000;
        int height = 1000;

        for (Aircraft* a : aircrafts) {
            int x = a->getPositionX() + n * a->getSpeedX();
            int y = a->getPositionY() + n * a->getSpeedY();
            int z = a->getPositionZ() + n * a->getSpeedZ();

            for (Aircraft* b : aircrafts) {
                if (a != b) {
                    int bx = b->getPositionX() + n * b->getSpeedX();
                    int by = b->getPositionY() + n * b->getSpeedY();
                    int bz = b->getPositionZ() + n * b->getSpeedZ();

                    if (bx > x + width) continue;
                    if (bx < x - width) continue;
                    if (by > y + width) continue;
                    if (by < y - width) continue;
                    if (bz > z + height) continue;
                    if (bz < z - height) continue;

                    output.append(to_string(a->getFlightId()) + " close to " + to_string(b->getFlightId()) + "\n");
                }
            }
        }
        if (output != "") {
            cout << "SEPARATION CONSTRAINT VIOLATION AT CURRENT TIME + " << n << " seconds " << endl;
            cout << output;
        }

        sleep(p);
    }
    name_detach(attach, 0); // NEVER REACHED
}

/**
 * Alarm Function
 */
void ComputerSystem::alarm() {
    int m = 180; // n = [0:180]

    string output = "";
    int width = 3000;
    int height = 1000;

    for (int n = 0; n < m; n++) { // Separation check for every second between 1 and 180 seconds in the future
        for (Aircraft* a : aircrafts) {
            int x = a->getPositionX() + n * a->getSpeedX();
            int y = a->getPositionY() + n * a->getSpeedY();
            int z = a->getPositionZ() + n * a->getSpeedZ();

            for (Aircraft* b : aircrafts) {
                if (a != b) {
                    int bx = b->getPositionX() + n * b->getSpeedX();
                    int by = b->getPositionY() + n * b->getSpeedY();
                    int bz = b->getPositionZ() + n * b->getSpeedZ();

                    if (bx > x + width) continue;
                    if (bx < x - width) continue;
                    if (by > y + width) continue;
                    if (by < y - width) continue;
                    if (bz > z + height) continue;
                    if (bz < z - height) continue;

                    output.append(to_string(a->getFlightId()) + " close to " + to_string(b->getFlightId()) + "\n");
                    n = m;
                }
            }
        }
        if (output != "") {
            cout << "Alarm: safety violation will happen within 3 minutes" << endl;
            cout << output;
        }
    }
}

ComputerSystem::~ComputerSystem() {}
