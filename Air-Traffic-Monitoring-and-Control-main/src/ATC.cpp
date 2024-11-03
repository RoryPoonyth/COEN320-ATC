#include <iostream>
#include "Aircraft.h"
#include "Radar.h"
#include "CommunicationSystem.h"
#include "DataDisplay.h"
#include "ComputerSystem.h"
#include "OperatorConsole.h"
#include <vector>
#include <fstream>

#define ROWS 25
#define COLUMNS 25

#define ATTACH_POINT "default"

using namespace std;

void createInput() { // Call createInput() if you need to modify the input file
    fstream file;
    file.open("low.txt", ios::out); // Relative path (you can find the file in /data/var/tmp in the virtual machine)
    if (file.is_open()) {
        file << "3, 0, 0, 0, 250, 250, 0, 0\n";
        file << "4, 5000, 0, 0, 250, 0, 0, 0\n";
        file << "5, 1800, 0, 0, 251, 0, 0, 0\n";
        file.close();
    }

    file.open("med.txt", ios::out);
    if (file.is_open()) {
        file << "3, 0, 0, 0, 250, 250, 0, 0\n";
        file << "4, 5000, 0, 0, 250, 0, 0, 0\n";
        file << "5, 1800, 0, 0, 251, 0, 0, 0\n";
        file << "6, 0, 700, 0, 300, 0, 0, 0\n";
        file << "7, 1000, 3000, 0, 500, 0, 0, 0\n";
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
        file.close();
    }
}

void printInput(const string& filename) { // Call printInput() if you need to verify that the input file was created
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

    vector<Aircraft*> aircrafts;

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

            aircrafts.push_back(new Aircraft(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]));
        }
        file.close();
    }

    computerSystem.setAircrafts(aircrafts);
    computerSystem.alarm();
    for (Aircraft* ap : aircrafts) {
        ap->startThread();
    }

    // Start the threads
    pthread_t mapDisplayThread, userInputThread, computerSystemThread, dataDisplayThread, mapInitializeThread, separationThread, radarThread;
    pthread_create(&radarThread, nullptr, &Radar::radarInitializer, &radar);
    pthread_create(&mapInitializeThread, nullptr, &DataDisplay::MapInitializer, &dataDisplay);
    pthread_create(&mapDisplayThread, nullptr, &ComputerSystem::MapDisplayThread, &computerSystem);
    pthread_create(&dataDisplayThread, nullptr, &DataDisplay::dataDisplayInitializer, &dataDisplay);
    pthread_create(&computerSystemThread, nullptr, &ComputerSystem::computerSystemThread, &computerSystem);
    pthread_create(&userInputThread, nullptr, &OperatorConsole::listenForOperatorInput, &operatorConsole);
    pthread_create(&separationThread, nullptr, &ComputerSystem::separationCheckThread, &computerSystem);

    // Run infinitely, otherwise main will complete its execution before the threads and program will terminate
    while (true) {
        sleep(500); // Reduce resource use
    }

    for (Aircraft* aircraft : aircrafts) {
        aircraft->stopThread();
    }

    return 0;
}
