# COEN320-ATC

## Introduction
This project is part of the COEN 320 course titled **Introduction to Real-Time Systems** in the Department of Electrical and Computer Engineering, Fall 2024. The project aims to develop an Air Traffic Monitoring and Control System that simulates the management of multiple aircraft in real-time, incorporating key concepts of real-time systems, such as concurrency, inter-process communication, and safety-critical operations.

## Project Overview
The Air Traffic Monitoring and Control System is designed to monitor and control the movement of multiple aircraft within a simulated airspace. The system handles tasks like aircraft tracking, collision avoidance, and responding to operator commands, using a combination of real-time techniques including multithreading, inter-process communication (IPC), and timer management.

### Key Components
1. **Aircraft**: Represents an individual aircraft with properties such as position, speed, and flight ID. Aircraft objects move in real-time and can receive commands from the operator to adjust speed or direction.

2. **ComputerSystem**: The core system managing aircraft information, handling operator commands, and ensuring safety through separation checks. The ComputerSystem is responsible for interacting with other components, including CommunicationSystem, Radar, and DataDisplay.

3. **Radar**: Collects data about aircraft positions and provides this information to the ComputerSystem for processing and display. Radar handles periodic requests to update the position of each aircraft.

4. **CommunicationSystem**: Manages the communication between the operator console and aircraft. It relays operator commands to adjust aircraft parameters like speed.

5. **DataDisplay**: Provides a visual representation of the current aircraft positions on a 2D map. The DataDisplay component updates periodically based on the information received from the ComputerSystem.

6. **OperatorConsole**: Provides the interface for an operator to interact with the system, such as requesting aircraft information or sending commands to change speed and altitude.

### Features
- **Real-Time Monitoring**: Tracks multiple aircraft and their positions, updating information in real-time.
- **Concurrency**: Uses multithreading to simulate the simultaneous movement of aircraft, handle operator commands, and monitor aircraft separation.
- **Inter-Process Communication (IPC)**: Employs QNX IPC mechanisms to communicate between system components, ensuring that information is transmitted efficiently and reliably.
- **Safety Checks**: Performs regular checks to prevent aircraft from violating safe separation distances. If a violation is predicted, an alarm is triggered.

## Requirements
- **QNX Neutrino RTOS**: The project runs on QNX Neutrino, a real-time operating system, to simulate realistic timing and IPC mechanisms.
- **Momentics IDE**: This project uses Momentics IDE for development and debugging.

## Build Instructions
1. Clone the repository to your local machine:
   ```bash
   git clone https://github.com/RoryPoonyth/COEN320_PROJECT
   ```
2. Open the project in **Momentics IDE**.
3. Build the project:
   - Select the **x86_64-debug** configuration.
   - Click **Build** or run `make -j12 all` in the terminal.
4. Run the executable within the QNX environment.

## Running the System
- Upon running, the system will ask for congestion level (low, medium, high) to determine the number of aircraft in the airspace.
- Operator commands can be entered to control aircraft speed or request specific aircraft information.

## Project Structure
- **src/**: Contains all source files for the project.
  - `Aircraft.cpp`, `Radar.cpp`, `ComputerSystem.cpp`, etc.
- **include/**: Header files for the project.
  - `Aircraft.h`, `Radar.h`, `ComputerSystem.h`, etc.
- **build/**: Contains the build output files.

## How It Works
- **Aircraft Movement**: Each aircraft moves in real-time and updates its position based on its speed and direction.
- **Concurrency and Synchronization**: Threads are used for concurrent tasks like moving aircraft, processing operator commands, and updating the 2D map.
- **Safety Monitoring**: The ComputerSystem component continuously monitors aircraft positions and calculates future positions to predict and avoid collisions.

## Future Improvements
- **Graphical User Interface (GUI)**: Implementing a more sophisticated GUI to enhance visualization.
- **Extended IPC Mechanisms**: Using more advanced IPC methods for better performance in high-congestion scenarios.
- **Enhanced Safety Protocols**: Adding predictive algorithms for earlier detection of potential collisions.

## Contributors
- Rory
- Omar
- Yazid

## License
This project is licensed under the MIT License. See the `LICENSE` file for more details.

