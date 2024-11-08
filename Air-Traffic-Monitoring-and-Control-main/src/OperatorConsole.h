#ifndef SRC_OPERATORCONSOLE_H_
#define SRC_OPERATORCONSOLE_H_

#include <string>
#include <vector>
#include <iostream>
#include <sys/dispatch.h>

using namespace std;

// The OperatorConsole class handles user input for managing aircraft information and logging commands
class OperatorConsole {
private:
	string log_entry = ""; // Stores log entries to record operator commands

public:
	// Constructor to initialize the OperatorConsole object
	OperatorConsole();

	// Static initializer function for creating a thread to listen for operator input
	static void* listenForOperatorInput(void* args);

	// Static initializer function for creating a thread to display specific aircraft data
	static void* displayAircraftDataThread(void* args);

	// Static initializer function for creating a thread to update an aircraft's speed
	static void* updateAircraftSpeedThread(void* args);

	// Function to continuously listen for user input and process commands
	void listenForUserInput();

	// Function to update the speed of an aircraft based on user input
	void* updateAircraftSpeed();

	// Function to display information about a specific aircraft
	void* displayAircraftData();

	// Destructor to clean up resources used by the OperatorConsole object
	virtual ~OperatorConsole();

	// Logs the operator commands to a file
	void writeLog(string log_entry);
};

#endif /* SRC_OPERATORCONSOLE_H_ */
