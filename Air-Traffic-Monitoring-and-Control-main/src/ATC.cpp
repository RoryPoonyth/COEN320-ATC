#include <iostream>
#include "Aircraft.h"
#include "Radar.h"
#include "CommunicationSystem.h"
#include "DataDisplay.h"
#include "ComputerSystem.h"
#include "OperatorConsole.h"
#include <vector>
#include <fstream>
#include <thread>
#include <memory>

#define ROWS 25
#define COLUMNS 25

#define ATTACH_POINT "default"

using namespace std;

void createInput() {
    fstream file;
    file.open("low.txt", ios::out);
    if (file.is_open()) {
        file << "3, 0, 0, 0, 250, 250, 0, 0\n";
        file << "4, 5000, 0, 0, 250, 0, 0, 0\n";
        file << "5, 1800, 0, 0, 251, 0, 0, 0\n";
        file << "10, 1000, 1000, 0, 200, 150, 0, 0\n";
        file.close();
    }

    file.open("med.txt", ios::out);
    if (file.is_open()) {
        file << "3, 0, 0, 0, 250, 250, 0, 0\n";
        file << "4, 5000, 0, 0, 250, 0, 0, 0\n";
        file << "5, 1800, 0, 0, 251, 0, 0, 0\n";
        file << "6, 0, 700, 0, 300, 0, 0, 0\n";
        file << "7, 1000, 3000, 0, 500, 0, 0, 0\n";
        file << "11, 1500, 2000, 0, 250, 100, 0, 0\n";
        file.close();
    }

    file.open("high.txt", ios::out);
    if (file.is_open()) {
        file << "3, 0, 0, 0, 250, 250, 0, 0\n";
        file << "4, 5000, 0, 0, 250, 0, 0, 0\n";
        file << "5, 1800, 0, 0, 251, 0, 0, 0\n";
        file << "6, 0, 700, 0, 300, 0, 0, 0\n";
        file << "7, 1000, 3000, 0, 500, 0, 0, 0\n";
        file << "8, 4000, 700, 0, 300, 0, 0, 0\n";
        file << "9, 1000, 200, 0, 500, 0, 0, 0\n";
        file << "12, 2000, 1500, 0, 400, 200, 0, 0\n";
        file << "13, 2500, 2500, 0, 450, 150, 0, 0\n";
        file.close();
    }
}

void printInput(const string& filename) {
    fstream file;
    file.open(filename, ios::in);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }
}

int main() {
    string filename = "";
    int congestion = 0;
    cout << "Type 1 for low, 2 for medium, 3 for high congestion: " << endl;
    cin >> congestion;
    switch (congestion) {
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
            cout << "Invalid option" << endl;
            return 1;
    }

    createInput();
    printInput(filename);

    DataDisplay dataDisplay;
    ComputerSystem computerSystem;
    OperatorConsole operatorConsole;
    Radar radar;
    CommunicationSystem communicationSystem;

    vector<unique_ptr<Aircraft>> aircrafts;

    fstream file;
    file.open(filename, ios::in);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            string delimiter = ",";
            size_t pos = 0;
            string token;
            vector<int> args;

            while ((pos = line.find(delimiter)) != string::npos) {
                token = line.substr(0, pos);
                args.push_back(stoi(token));
                line.erase(0, pos + delimiter.length());
            }
            args.push_back(stoi(line));

            aircrafts.push_back(std::unique_ptr<Aircraft>(new Aircraft(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7])));
        }
        file.close();
    }

    // Create a vector of raw pointers to pass to ComputerSystem
    vector<Aircraft*> aircraftPointers;
    for (auto& aircraft : aircrafts) {
        aircraftPointers.push_back(aircraft.get());
    }

    computerSystem.setAircrafts(aircraftPointers);
    computerSystem.alarm();

    for (auto& aircraft : aircrafts) {
        aircraft->startThread();
    }

    // Create and start threads using std::thread
    vector<std::thread> threads;

    threads.emplace_back(&Radar::radarInitializer, &radar);
    threads.emplace_back(&DataDisplay::MapInitializer, &dataDisplay);
    threads.emplace_back(&ComputerSystem::MapDisplayThread, &computerSystem);
    threads.emplace_back(&DataDisplay::dataDisplayInitializer, &dataDisplay);
    threads.emplace_back(&ComputerSystem::computerSystemThread, &computerSystem);
    threads.emplace_back(&OperatorConsole::listenForOperatorInput, &operatorConsole);
    threads.emplace_back(&ComputerSystem::separationCheckThread, &computerSystem);

    // Run infinitely, otherwise main will complete its execution before the threads and program will terminate
    while (true) {
        this_thread::sleep_for(chrono::seconds(500)); // Reduce resource use
    }

    // Stop aircraft threads
    for (auto& aircraft : aircrafts) {
        aircraft->stopThread();
    }

    // Join all threads
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}
