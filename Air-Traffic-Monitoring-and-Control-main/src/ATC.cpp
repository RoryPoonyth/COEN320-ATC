#include <iostream>

#include "Aircraft.h"
#include "Radar.h"
#include "CommunicationSystem.h"
#include "DataDisplay.h"
#include "ComputerSystem.h"
#include "OperatorConsole.h"
#include <vector>
#include <fstream>

#define rows 25 // Defines grid dimensions for display or calculations
#define columns 25

#define ATTACH_POINT "default" // Placeholder for attachment point configuration


using namespace std; // Using the standard namespace

// Function to create sample input files for different congestion levels
void createInput(){
	fstream file;

	// Creates "low.txt" for low congestion data
	file.open("low.txt", ios::out); // File saved in /data/var/tmp in virtual machine
	if(file.is_open()){
		// Format: flightId, positionX, positionY, positionZ, speedX, speedY, speedZ, time
		file << "1, 0, 0, 0, 250, 250, 0, 0 \n";
		file << "2, 5000, 0, 0, 250, 0, 0, 0 \n";
		file << "3, 1800, 0, 0, 251, 0, 0, 0 \n";
		// Add more aircraft data if needed
		file.close();
	}

	// Creates "med.txt" for medium congestion data
	file.open("med.txt", ios::out);
	if(file.is_open()){
		file << "1, 0, 0, 0, 250, 250, 0, 0 \n";
		file << "2, 5000, 0, 0, 250, 0, 0, 0 \n";
		file << "3, 1800, 0, 0, 251, 0, 0, 0 \n";
		file << "4, 0, 700, 0, 300, 0, 0, 0 \n";
		file << "5, 1000, 3000, 0, 500, 0, 0, 0 \n";
		// Add more aircraft data if needed
		file.close();
	}

	// Creates "high.txt" for high congestion data
	file.open("high.txt", ios::out);
	if(file.is_open()){
		file << "1, 0, 0, 0, 250, 250, 0, 0 \n";
		file << "2, 5000, 0, 0, 250, 0, 0, 0 \n";
		file << "3, 1800, 0, 0, 251, 0, 0, 0 \n";
		file << "4, 0, 700, 0, 300, 0, 0, 0 \n";
		file << "5, 1000, 3000, 0, 500, 0, 0, 0 \n";
		file << "6, 4000, 700, 0, 300, 0, 0, 0 \n";
		file << "7, 1000, 200, 0, 500, 0, 0, 0 \n";
		// Add more aircraft data if needed
		file.close();
	}
}

// Function to print the contents of a specified file
void printInput(string filename){
	fstream file;
	file.open(filename, ios::in); // Opens the file for reading
	if(file.is_open()){
		string line;
		while(getline(file, line)){
			cout << line << endl; // Outputs each line
		}
		file.close();
	}
}

int main() {
	string filename = ""; // Holds the filename based on congestion level
	int congestion = 0;

	cout << "type 1 for low, 2 for med, 3 for high congestion" << endl;
	cin >> congestion;

	// Sets the filename based on user input
	switch(congestion){
	case 1:
		filename = "low.txt";
		break;
	case 2:
		filename = "med.txt";
		break;
	case 3:
		filename = "high.txt";
		break;
	}

	createInput(); // Creates input files
	printInput(filename); // Verifies content by printing the selected file

	// System components initialization
	DataDisplay dataDisplay;
	ComputerSystem computerSystem;
	OperatorConsole operatorConsole;
	Radar radar;
	CommunicationSystem communicationSystem;

	vector<Aircraft*> aircrafts; // Vector to hold Aircraft objects

	// Load aircraft data from file and populate aircrafts vector
	fstream file;
	file.open(filename, ios::in);
	if(file.is_open()){
		string line;
		while(getline(file, line)){
			string delimiter = ",";
			size_t pos = 0;
			string token;
			vector<int> arg; // Holds parsed values for an aircraft

			// Parse each value from line and add to arg vector
			while ((pos = line.find(delimiter)) < 10000000) {
			    token = line.substr(0, pos);
			    arg.push_back((int)atoi(token.c_str()));
			    line.erase(0, pos + delimiter.length());
			}
			arg.push_back((int)atoi(line.c_str())); // Adds last element

			// Create a new Aircraft object with parsed values and add to vector
			aircrafts.push_back(new Aircraft(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7]));
		}
		file.close();
	}

	// Assign aircrafts to computer system and initiate alarm
	computerSystem.setAircrafts(aircrafts);
	computerSystem.alarm();

	// Start each aircraft's thread
	for(Aircraft* ap : aircrafts){
		ap->startThread();
	}

	// Create and initialize system threads for various components
	pthread_t mapDisplayThread, userInputThread, computerSystemThread, dataDisplayThread, mapInitializeThread, separationThread, radarThread;

	pthread_create(&radarThread, nullptr, &Radar::radarInitializer, &radar);
	pthread_create(&mapInitializeThread, nullptr, &DataDisplay::MapInitializer, &dataDisplay);
	pthread_create(&mapDisplayThread, nullptr, &ComputerSystem::MapDisplayThread, &computerSystem);
	pthread_create(&dataDisplayThread, nullptr, &DataDisplay::dataDisplayInitializer, &dataDisplay);
	pthread_create(&computerSystemThread, nullptr, &ComputerSystem::computerSystemThread, &computerSystem);
	pthread_create(&userInputThread, nullptr, &OperatorConsole::listenForOperatorInput, &operatorConsole);
	pthread_create(&separationThread, nullptr, &ComputerSystem::separationCheckThread, &computerSystem);

	// Main loop to keep the program running indefinitely
	while (true) {
		sleep(500); // Reduce resource use by adding a delay
	}

	// Stop each aircraft's thread before exiting
	for (size_t i = 0; i < aircrafts.size(); ++i) {
		aircrafts[i]->stopThread();
	}

	return 0;
}
