#include "ComputerSystem.h"
#include "CommunicationSystem.h"
#include "Radar.h"
#include "Aircraft.h"
#include "ctimer.h"
#include <sys/dispatch.h>
#include <vector>
#include <string>
#include <thread>
#include <memory>
#include <chrono>
#include <iostream>

#define ATTACH_POINT "default"

ComputerSystem::ComputerSystem() {}

void ComputerSystem::setAircrafts(const std::vector<Aircraft*>& aircrafts) {
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
    ComputerSystem* computerSystem = static_cast<ComputerSystem*>(args);
    computerSystem->MapDisplay();
    return nullptr;
}

void* ComputerSystem::computerSystemThread(void* args) {
    ComputerSystem* computerSystem = static_cast<ComputerSystem*>(args);
    computerSystem->computerSystemOperations();
    return nullptr;
}

void* ComputerSystem::separationCheckThread(void* args) {
    ComputerSystem* computerSystem = static_cast<ComputerSystem*>(args);
    computerSystem->separationCheck();
    return nullptr;
}

/**
 * Function to Display 2D Map
 */
void* ComputerSystem::MapDisplay() {
    while (true) {
        std::vector<AircraftData> aircraftsInfo;
        Radar radar;

        // Vector for managing listen threads
        std::vector<std::thread> threads;

        // Create the Aircraft listen threads for each aircraft
        for (auto& aircraft : aircrafts) {
            std::string attachPointRAD = std::string(ATTACH_POINT) + "_" + std::to_string(aircraft->getFlightId());
            threads.emplace_back(&Aircraft::aircraftListenHelper, new AircraftListenArgs{aircraft, attachPointRAD});
        }

        // Store received aircraft info into aircraftsInfo vector
        for (auto& aircraft : aircrafts) {
            // Replacing the incorrect method name with the correct one
            AircraftData aircraftResult = radar.requestPingForAircraft(aircraft->getFlightId());
            aircraftsInfo.push_back(aircraftResult);
        }

        // Join all listen threads created during this period
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        // Set header type and subtype for communication with DataDisplay
        aircraftsInfo[0].header.type = 0x05;
        aircraftsInfo[0].header.subtype = 0x05;

        std::string attachPointDataDisplay = std::string(ATTACH_POINT) + "_MAP";

        // Establish connection (communication) with DataDisplay
        int coid;
        if ((coid = name_open(attachPointDataDisplay.c_str(), 0)) == -1) {
            perror("ComputerSystem (MapDisplay): Error occurred while attaching the channel");
            return reinterpret_cast<void*>(EXIT_FAILURE);
        }

        // Send aircrafts info to DataDisplay for Map, expect no response
        if (MsgSend(coid, &aircraftsInfo, sizeof(aircraftsInfo), nullptr, 0) == -1) {
            std::cout << "Error while sending the message: " << strerror(errno) << std::endl;
            name_close(coid);
            return reinterpret_cast<void*>(EXIT_FAILURE);
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return reinterpret_cast<void*>(EXIT_SUCCESS);
}

/**
 * Computer System Operations based on OperatorConsole requests
 * 1. Update Aircraft Speed (via CommunicationSystem)
 * 2. Return Aircraft Information (via Radar)
 */
void ComputerSystem::computerSystemOperations() {
    name_attach_t* attach;

    if ((attach = name_attach(nullptr, ATTACH_POINT, 0)) == nullptr) {
        perror("Error occurred while creating the attach point sendNewAircraftSpeedToCommunicationSystem");
        return;
    }

    int newSpeedX, newSpeedY, newSpeedZ;
    AircraftData aircraftCommand;

    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), nullptr);

        if (rcvid == -1) { // Error condition, exit
            break;
        }

        if (aircraftCommand.header.type == 0x02 && aircraftCommand.header.subtype == 0x00) {
            newSpeedX = aircraftCommand.speedX;
            newSpeedY = aircraftCommand.speedY;
            newSpeedZ = aircraftCommand.speedZ;
            MsgReply(rcvid, EOK, nullptr, 0);

            Aircraft* aircraft = getAircraftByFlightId(aircraftCommand.flightId);
            if (!aircraft) {
                std::cout << "Error: Aircraft with flight ID " << aircraftCommand.flightId << " was not found." << std::endl;
                continue;
            }

            CommunicationSystem communicationSystem;
            communicationSystem.relayNewSpeed(*aircraft, newSpeedX, newSpeedY, newSpeedZ);
            alarm();
            continue;

        } else if (aircraftCommand.header.type == 0x02 && aircraftCommand.header.subtype == 0x01) {
            MsgReply(rcvid, EOK, nullptr, 0);

            Aircraft* aircraft = getAircraftByFlightId(aircraftCommand.flightId);
            if (!aircraft) {
                std::cout << "Error: Aircraft with flight ID " << aircraftCommand.flightId << " was not found." << std::endl;
                continue;
            }

            // Create the attach point for radar
            std::string radarAttachPoint = std::string(ATTACH_POINT) + "_RADAR";

            int radarCOID;
            if ((radarCOID = name_open(radarAttachPoint.c_str(), 0)) == -1) {
                perror("Computer System (computerSystemOperations): Error occurred while attaching the channel for Radar Request");
                return;
            }

            aircraftCommand.header.type = 0x08;
            aircraftCommand.header.subtype = 0x08;

            // Send data to Radar expecting response with Aircraft Information
            if (MsgSend(radarCOID, &aircraftCommand, sizeof(aircraftCommand), &aircraftCommand, sizeof(aircraftCommand)) == -1) {
                std::cout << "Error while sending the message for aircraft: " << strerror(errno) << std::endl;
                name_close(radarCOID);
            }

            // Send the received data to DataDisplay
            std::string attachPointDATADISPLAY = std::string(ATTACH_POINT) + "_datadisplay_";

            int coid;
            if ((coid = name_open(attachPointDATADISPLAY.c_str(), 0)) == -1) {
                perror("Error occurred while attaching the channel IN COMPSYS FOR DATADISPLAY");
                return;
            }

            aircraftCommand.header.type = 0x05;
            aircraftCommand.header.subtype = 0x01;

            // Send data to DataDisplay expecting no response
            if (MsgSend(coid, &aircraftCommand, sizeof(aircraftCommand), nullptr, 0) == -1) {
                std::cout << "Error while sending the message for aircraft: " << strerror(errno) << std::endl;
                name_close(coid);
                return;
            }
        }
    }
    name_detach(attach, 0);
}

/**
 * Separation Check Function
 */
void ComputerSystem::separationCheck() {
    name_attach_t *attach;

    std::string attachPoint = std::string(ATTACH_POINT) + "_separation";

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
        for (int i = 0; i < 2; i++) {
            if ((rcvid = MsgReceive(attach->chid, &separationCommand, sizeof(separationCommand), NULL)) != -1) {
                n = separationCommand.n_seconds;
                timer.setTimerSpec(separationCommand.p_interval, 0);
                std::cout << "Message Received N: " << n << " P:" << separationCommand.p_interval << std::endl;
                MsgReply(rcvid, EOK, NULL, 0);
                i = 200;
            }
        }

        std::string output = "";
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

                    if (bx > x + width || bx < x - width || by > y + width || by < y - width || bz > z + height || bz < z - height) {
                        continue;
                    }

                    output.append("" + std::to_string(a->getFlightId()) + " close to " + std::to_string(b->getFlightId()) + "\n");
                }
            }
        }
        if (output != "") {
            std::cout << "SEPARATION CONSTRAINT VIOLATION AT CURRENT TIME + " << n << " seconds " << std::endl;
            std::cout << output;
        }

        sleep(p);
    }
    name_detach(attach, 0);
}

/**
 * Alarm Function
 */
void ComputerSystem::alarm() {
    int m = 180;

    std::string output = "";
    int width = 3000;
    int height = 1000;

    for (int n = 0; n < m; n++) {
        for (Aircraft* a : aircrafts) {
            int x = a->getPositionX() + n * a->getSpeedX();
            int y = a->getPositionY() + n * a->getSpeedY();
            int z = a->getPositionZ() + n * a->getSpeedZ();

            for (Aircraft* b : aircrafts) {
                if (a != b) {
                    int bx = b->getPositionX() + n * b->getSpeedX();
                    int by = b->getPositionY() + n * b->getSpeedY();
                    int bz = b->getPositionZ() + n * b->getSpeedZ();

                    if (bx > x + width || bx < x - width || by > y + width || by < y - width || bz > z + height || bz < z - height) {
                        continue;
                    }

                    output.append("" + std::to_string(a->getFlightId()) + " close to " + std::to_string(b->getFlightId()) + "\n");
                    n = m;
                }
            }
        }
        if (output != "") {
            std::cout << "Alarm: safety violation will happen within 3 minutes" << std::endl;
            std::cout << output;
        }
    }
}

ComputerSystem::~ComputerSystem() {}
