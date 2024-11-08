#ifndef SRC_RADAR_H_
#define SRC_RADAR_H_

#include "Aircraft.h"
#include <vector>

using namespace std;

// Radar class: Responsible for tracking aircraft positions and relaying their data
class Radar {
public:
	// Default constructor to initialize a Radar object
	Radar();

	// Constructor to initialize Radar with a list of Aircraft objects
	Radar(vector<Aircraft> &aircrafts);

	// Constructor to initialize Radar with a single Aircraft object
	Radar(Aircraft &aircraft);

	// Static initializer function for creating a thread to start radar operations
	static void* radarInitializer(void* args);

	// Continuously pings aircraft to request their current position and speed
	void* pingAircraft();

	// Requests the position and speed of a specific aircraft based on its flight ID
	AircraftData operatorRequestPingAircraft(int flightId);

	// Destructor to clean up resources used by the Radar object
	virtual ~Radar();

private:
	Aircraft aircraft; // Single Aircraft object for individual tracking
	vector<Aircraft> aircrafts; // Vector of Aircraft objects for group tracking
};

#endif /* SRC_RADAR_H_ */
