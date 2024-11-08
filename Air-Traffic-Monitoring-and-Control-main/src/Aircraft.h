#include <pthread.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/neutrino.h>
#ifndef SRC_AIRCRAFT_H_
#define SRC_AIRCRAFT_H_

using namespace std;

// Data structure to store aircraft information, including position and speed
typedef struct {
    struct _pulse header;  // Pulse header for receiving messages
    int flightId;          // Unique identifier for the flight
    int positionX;         // Current X position of the aircraft
    int positionY;         // Current Y position of the aircraft
    int positionZ;         // Current Z position of the aircraft

    int speedX;            // Speed along the X axis
    int speedY;            // Speed along the Y axis
    int speedZ;            // Speed along the Z axis
} AircraftData;

// Aircraft class definition
class Aircraft {
private:
    int flightId = 0;        // Unique identifier for the aircraft
    int positionX = 0;       // Current X position
    int positionY = 0;       // Current Y position
    int positionZ = 0;       // Current Z position
    int speedX = 0;          // Speed along the X axis
    int speedY = 0;          // Speed along the Y axis
    int speedZ = 0;          // Speed along the Z axis
    int time = 0;            // Timestamp or interval (type can be discussed further)
    int err_no = 0;          // Error number for thread operations
    pthread_attr_t attr;     // Thread attributes
    pthread_t thread;        // Thread handle

public:
    // Constructors and destructor
    Aircraft();
    Aircraft(int flightId, int positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time);
    virtual ~Aircraft();

    // Getter and setter methods for flight ID
    int getFlightId();
    void setFlightId(int flightId);

    // Getter and setter methods for position coordinates
    int getPositionX();
    int getPositionY();
    int getPositionZ();
    int getPrevPositionX();
    int getPrevPositionY();
    int getPrevPositionZ();
    void setPositionX(int positionX);
    void setPositionY(int positionY);
    void setPositionZ(int positionZ);

    // Getter and setter methods for speed in X, Y, Z directions
    int getSpeedX();
    int getSpeedY();
    int getSpeedZ();
    void setSpeedX(int speedX);
    void setSpeedY(int speedY);
    void setSpeedZ(int speedZ);

    // Methods to start and stop the aircraft operations thread
    void startThread();
    void stopThread();

    // Static function for plane operations, executed in a thread
    static void *planeOperations(void *arg);

    // Static helper function for aircraft listening
    static void* aircraftListenHelper(void* args);

    // Function to move the aircraft based on current speed
    void move();

    // Function to listen for incoming commands via attach point
    void* listen(string attachPoint);
};

// Structure to hold arguments for the aircraftListenHelper function
struct AircraftListenArgs {
    Aircraft* aircraft;        // Pointer to the Aircraft instance
    string attachPoint;        // Attach point for listening
};

#endif /* SRC_AIRCRAFT_H_ */
