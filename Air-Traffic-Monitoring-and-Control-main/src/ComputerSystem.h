#ifndef SRC_COMPUTERSYSTEM_H_
#define SRC_COMPUTERSYSTEM_H_

#include "Aircraft.h"
#include <vector>

using namespace std;

// Structure to store data for separation checks
typedef struct {
	int n_seconds; // Number of seconds to check ahead for separation
	int p_interval; // Interval period for checking separation
} SeparationData;

// The ComputerSystem class manages aircraft operations, communication, and monitoring
class ComputerSystem {
public:
	// Constructor to initialize the ComputerSystem object
	ComputerSystem();

	// Static wrapper functions to start threads for different system components
	static void* MapDisplayThread(void* args);
	static void* dataDisplayThread(void* args);
	static void* computerSystemThread(void* args);
	static void* separationCheckThread(void* args);

	// Displays the 2D map of aircraft positions
	void* MapDisplay();

	// Sets the list of aircrafts managed by the system
	void setAircrafts(vector<Aircraft*> aircrafts);

	// Retrieves an Aircraft object based on its flight ID
	Aircraft* getAircraftByFlightId(int flightId);

	// Sends a new speed command to an aircraft via the communication system
	void sendNewAircraftSpeedToCommunicationSystem();

	// Requests specific aircraft data as per operator request
	void operatorRequestAircraftData();

	// Handles operations based on requests from the Operator Console
	void computerSystemOperations();

	// Checks aircraft separation to ensure safe distances
	void separationCheck();

	// Issues an alarm if any safety violations are detected
	void alarm();

	// Destructor to clean up resources used by the ComputerSystem object
	virtual ~ComputerSystem();

private:
	vector<Aircraft*> aircrafts; // Vector to hold pointers to managed Aircraft objects
};

#endif /* SRC_COMPUTERSYSTEM_H_ */
