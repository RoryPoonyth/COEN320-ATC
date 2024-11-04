#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <chrono>
#include "Aircraft.h"
#include "Radar.h"
#include "CommunicationSystem.h"
#include "DataDisplay.h"
#include "ComputerSystem.h"
#include "OperatorConsole.h"

constexpr int ROWS = 25;
constexpr int COLUMNS = 25;
constexpr char ATTACH_POINT[] = "default";

using namespace std;
using namespace std::chrono_literals;

void createInputFile(const string& filename, const vector<string>& data) {
    ofstream file(filename);
    if (!file) {
        cerr << "Error creating file: " << filename << endl;
        return;
    }
    for (const auto& line : data) {
        file << line << '\n';
    }
}

void createInputFiles() {
    createInputFile("low.txt", {
        "3, 0, 0, 0, 250, 250, 0, 0",
        "4, 5000, 0, 0, 250, 0, 0, 0",
        "5, 1800, 0, 0, 251, 0, 0, 0"
    });

    createInputFile("med.txt", {
        "3, 0, 0, 0, 250, 250, 0, 0",
        "4, 5000, 0, 0, 250, 0, 0, 0",
        "5, 1800, 0, 0, 251, 0, 0, 0",
        "6, 0, 700, 0, 300, 0, 0, 0",
        "7, 1000, 3000, 0, 500, 0, 0, 0"
    });

    createInputFile("high.txt", {
        "3, 0, 0, 0, 250, 250, 0, 0",
        "4, 5000, 0, 0, 250, 0, 0, 0",
        "5, 1800, 0, 0, 251, 0, 0, 0",
        "6, 0, 700, 0, 300, 0, 0, 0",
        "7, 1000, 3000, 0, 500, 0, 0, 0",
        "8, 4000, 700, 0, 300, 0, 0, 0",
        "9, 1000, 200, 0, 500, 0, 0, 0"
    });
}

void printFileContent(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }
    string line;
    while (getline(file, line)) {
        cout << line << endl;
    }
}

vector<unique_ptr<Aircraft>> loadAircraftsFromFile(const string& filename) {
    ifstream file(filename);
    vector<unique_ptr<Aircraft>> aircrafts;

    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return aircrafts;
    }

    string line;
    while (getline(file, line)) {
        vector<int> args;
        size_t pos;
        while ((pos = line.find(',')) != string::npos) {
            args.push_back(stoi(line.substr(0, pos)));
            line.erase(0, pos + 1);
        }
        args.push_back(stoi(line));

        if (args.size() == 8) {
            aircrafts.push_back(make_unique<Aircraft>(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]));
        } else {
            cerr << "Invalid line in file: " << line << endl;
        }
    }
    return aircrafts;
}

int main() {
    int congestion = 0;
    cout << "Select congestion level (1 = Low, 2 = Medium, 3 = High): ";
    cin >> congestion;

    string filename;
    switch (congestion) {
        case 1: filename = "low.txt"; break;
        case 2: filename = "med.txt"; break;
        case 3: filename = "high.txt"; break;
        default:
            cerr << "Invalid option" << endl;
            return 1;
    }

    createInputFiles();
    printFileContent(filename);

    DataDisplay dataDisplay;
    ComputerSystem computerSystem;
    OperatorConsole operatorConsole;
    Radar radar;
    CommunicationSystem communicationSystem;

    auto aircrafts = loadAircraftsFromFile(filename);
    computerSystem.setAircrafts(aircrafts);
    computerSystem.alarm();

    vector<thread> threads;
    threads.emplace_back(&Radar::radarInitializer, &radar);
    threads.emplace_back(&DataDisplay::MapInitializer, &dataDisplay);
    threads.emplace_back(&ComputerSystem::MapDisplayThread, &computerSystem);
    threads.emplace_back(&DataDisplay::dataDisplayInitializer, &dataDisplay);
    threads.emplace_back(&ComputerSystem::computerSystemThread, &computerSystem);
    threads.emplace_back(&OperatorConsole::listenForOperatorInput, &operatorConsole);
    threads.emplace_back(&ComputerSystem::separationCheckThread, &computerSystem);

    // Start aircraft threads
    for (auto& aircraft : aircrafts) {
        threads.emplace_back(&Aircraft::startThread, aircraft.get());
    }

    // Run infinitely with periodic delay to reduce resource use
    while (true) {
        this_thread::sleep_for(500ms);
    }

    // Clean up and stop all threads (not reached in this infinite loop)
    for (auto& aircraft : aircrafts) {
        aircraft->stopThread();
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}
