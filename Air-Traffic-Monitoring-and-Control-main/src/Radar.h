#pragma once

#include "Aircraft.h"
#include <vector>

class Radar {
public:
    Radar();
    Radar(const std::vector<Aircraft>& aircrafts);
    Radar(const Aircraft& aircraft);

    // Initializes radar and starts pinging aircraft
    void radarInitializer();

    // Pings aircraft in the system
    void pingAircraft();

    // Requests a ping for a specific aircraft by flight ID
    AircraftData requestPingForAircraft(int flightId);

    ~Radar();

private:
    Aircraft aircraft;
    std::vector<Aircraft> aircrafts;
};
