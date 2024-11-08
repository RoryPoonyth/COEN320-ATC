#pragma once

#include "Aircraft.h"
#include <vector>
#include <atomic>

class Radar {
public:
    Radar();
    Radar(const std::vector<Aircraft*>& aircrafts);
    Radar(Aircraft* aircraft);

    // Initializes radar and starts pinging aircraft
    static void* radarInitializer(void* args); // Static function used as a thread entry point

    // Pings aircraft in the system
    void pingAircraft();

    // Requests a ping for a specific aircraft by flight ID
    AircraftData requestPingForAircraft(int flightId) const;

    ~Radar();

    // Delete copy constructor and assignment operator
    Radar(const Radar&) = delete;
    Radar& operator=(const Radar&) = delete;

private:
    Aircraft* aircraft = nullptr; // Use a pointer instead of a direct object to avoid copying
    std::vector<Aircraft*> aircrafts; // Store a list of pointers to Aircraft objects

    void handlePulse(const AircraftData& aircraftCommand) const;
    void handleAircraftRequest(int rcvid, AircraftData& aircraftCommand) const;
};
