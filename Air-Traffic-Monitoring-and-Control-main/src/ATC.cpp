#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include <mutex>
#include <atomic>
#include "Aircraft.h"
#include "Radar.h"
#include "CommunicationSystem.h"
#include "DataDisplay.h"
#include "ComputerSystem.h"
#include "OperatorConsole.h"

#define rows 100
#define columns 100

#define ATTACH_POINT "default" // Placeholder for attachment point configuration


using namespace std; // Using the standard namespace

std::mutex aircraftsMutex;

std::atomic<bool> shutdownFlag(false);

// Function to create sample input files for different congestion levels
void createInput(){
	fstream file;

	// Creates "low.txt" for low congestion data
	file.open("low.txt", ios::out); // File saved in /data/var/tmp in virtual machine
	if(file.is_open()){
		// Format: flightId, positionX, positionY, positionZ, speedX, speedY, speedZ, time
		file << "1, 0, 0, 0, 250, 250, 0, 20 \n";
		file << "2, 5000, 0, 0, 250, 0, 0, 2 \n";
		file << "3, 1800, 0, 0, 251, 0, 0, 5 \n";
		// Add more aircraft data if needed
		file.close();
	}

	// Creates "med.txt" for medium congestion data
	file.open("med.txt", ios::out);
	if(file.is_open()){
		file << "1, 0, 0, 0, 250, 250, 0, 6 \n";
		file << "2, 5000, 0, 0, 250, 0, 0, 0 \n";
		file << "3, 1800, 0, 0, 251, 0, 0, 0 \n";
		file << "4, 0, 700, 0, 300, 0, 0, 3 \n";
		file << "5, 1000, 3000, 0, 500, 0, 0, 1 \n";
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

// Function to release aircrafts based on their release times
void releaseAircrafts(vector<Aircraft*>& aircrafts, chrono::steady_clock::time_point programStart) {
    for(auto& ap : aircrafts){
        int releaseTime = ap->getReleaseTime(); // Assuming releaseTime is in seconds

        // Calculate the time point when this aircraft should be released
        auto releasePoint = programStart + chrono::seconds(releaseTime);

        // Calculate the remaining time to wait
        std::this_thread::sleep_until(releasePoint);

        // Start the aircraft's thread
        {
            std::lock_guard<std::mutex> lock(aircraftsMutex);
            ap->startThread();
            cout << "Started thread for Aircraft ID: " << ap->getFlightId()
                 << " at time: " << releaseTime << " seconds." << endl;
        }
    }
}

// Function to listen for shutdown command
void listenForShutdown() {
    std::string input;
    while (true) {
        std::cin >> input;
        if (input == "shutdown") {
            shutdownFlag.store(true);
            break;
        }
    }
}

int main() {
    string filename = "";
    int congestion = 0;

    cout << "Type 1 for low, 2 for med, 3 for high congestion: ";
    cin >> congestion;

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
    default:
        cout << "Invalid input. Exiting." << endl;
        return 1;
    }

    createInput();
    printInput(filename);

    // Initialize system components
    DataDisplay dataDisplay;
    ComputerSystem computerSystem;
    OperatorConsole operatorConsole;
    Radar radar;
    CommunicationSystem communicationSystem;

    vector<Aircraft*> aircrafts;

    // Load aircraft data from file
    fstream file;
    file.open(filename, ios::in);
    if(file.is_open()){
        string line;
        while(getline(file, line)){
            string delimiter = ",";
            size_t pos = 0;
            string token;
            vector<int> arg;

            while ((pos = line.find(delimiter)) != string::npos) {
                token = line.substr(0, pos);
                arg.push_back(atoi(token.c_str()));
                line.erase(0, pos + delimiter.length());
            }
            arg.push_back(atoi(line.c_str())); // Last element

            // Create Aircraft object (ensure constructor matches)
            aircrafts.push_back(new Aircraft(arg[0], arg[1], arg[2], arg[3],
                                            arg[4], arg[5], arg[6], arg[7]));
        }
        file.close();
    } else {
        cout << "Failed to open file: " << filename << endl;
        return 1;
    }

    // Sort aircrafts by release time
    sort(aircrafts.begin(), aircrafts.end(), [](const Aircraft* a, const Aircraft* b) {
        return a->getReleaseTime() < b->getReleaseTime();
    });

    // Assign aircrafts to computer system and initiate alarm
    computerSystem.setAircrafts(aircrafts);
    computerSystem.alarm();

    // Record the program start time
    auto programStart = chrono::steady_clock::now();

    // Start the aircraft release thread
    std::thread aircraftReleaseThread(releaseAircrafts, std::ref(aircrafts), programStart);

    // Start system threads for various components immediately
    pthread_t mapDisplayThread, userInputThread, computerSystemThread, dataDisplayThread,
              mapInitializeThread, separationThread, radarThread;

    pthread_create(&radarThread, nullptr, &Radar::radarInitializer, &radar);
    pthread_create(&mapInitializeThread, nullptr, &DataDisplay::MapInitializer, &dataDisplay);
    pthread_create(&mapDisplayThread, nullptr, &ComputerSystem::MapDisplayThread, &computerSystem);
    pthread_create(&dataDisplayThread, nullptr, &DataDisplay::dataDisplayInitializer, &dataDisplay);
    pthread_create(&computerSystemThread, nullptr, &ComputerSystem::computerSystemThread, &computerSystem);
    pthread_create(&userInputThread, nullptr, &OperatorConsole::listenForOperatorInput, &operatorConsole);
    pthread_create(&separationThread, nullptr, &ComputerSystem::separationCheckThread, &computerSystem);

    // Start the map updating thread (assuming DataDisplay handles it)
    // Ensure that the map updating thread accesses aircrafts in a thread-safe manner

    // Optionally, detach the aircraft release thread if you don't need to join it
    aircraftReleaseThread.detach();

    // Main loop to keep the program running indefinitely
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Use C++ thread sleep
    }

    for (auto& aircraft : aircrafts) {
        aircraft->stopThread();
    }

    return 0;
}
