#include "Aircraft.h"
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <sys/dispatch.h>
#include <vector>
#include <string>
#include "cTimer.h"

using namespace std;

#define ATTACH_POINT "default"

// Default constructor for Aircraft class
Aircraft::Aircraft(){
    // Initialize thread to 0 (not yet created)
    this->thread = 0;
}

// Parameterized constructor for Aircraft class
Aircraft::Aircraft(int flightId, int positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time){
    // Initialize thread to 0 (not yet created)
    this->thread = 0;

    // Set initial values for flight information and position
    this->flightId = flightId;
    this->positionX = positionX;
    this->positionY = positionY;
    this->positionZ = positionZ;
    this->speedX = speedX;
    this->speedY = speedY;
    this->speedZ = speedZ;
    this->time = time;

    // Initialize thread attributes with default values
    err_no = pthread_attr_init(&attr);
    if (err_no!=0){
        printf("ERROR from pthread_attr_init() is %d \n", err_no);
    }

    // Set the thread detach state attribute to joinable
    err_no = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (err_no!=0){
        printf("ERROR from pthread_attr_setdetachstate() is %d \n", err_no);
    }

    // Set the stack size for the thread
    err_no= pthread_attr_setstacksize(&attr, 256);
    if (err_no!=0){
        printf("ERROR from pthread_attr_setstacksize() is %d \n", err_no);
    }
}

// Helper function to listen for incoming commands for the aircraft
// Used to unpack arguments and call the listen function with attachPoint
void* Aircraft::aircraftListenHelper(void* args) {
    AircraftListenArgs* arguments = (AircraftListenArgs*)args;
    return arguments->aircraft->listen(arguments->attachPoint);
}

// Destructor for Aircraft class
Aircraft::~Aircraft() {
    // No dynamic memory to free in current implementation
}

// Getter methods for flightId and position coordinates
int Aircraft::getFlightId(){
    return flightId;
}

int Aircraft::getPositionX(){
    return positionX;
}

int Aircraft::getPositionY(){
    return positionY;
}

int Aircraft::getPositionZ(){
    return positionZ;
}

// Get previous position coordinates based on current speed
int Aircraft::getPrevPositionX(){
    return positionX-speedX;
}

int Aircraft::getPrevPositionY(){
    return positionY-speedY;
}

int Aircraft::getPrevPositionZ(){
    return positionZ-speedZ;
}

// Getter methods for speed in X, Y, Z directions
int Aircraft::getSpeedX(){
    return speedX;
}

int Aircraft::getSpeedY(){
    return speedY;
}

int Aircraft::getSpeedZ(){
    return speedZ;
}

// Setter methods for flightId and position coordinates
void Aircraft::setFlightId(int flightId){
    this->flightId = flightId;
}

void Aircraft::setPositionX(int positionX){
    this->positionX = positionX;
}

void Aircraft::setPositionY(int positionY){
    this->positionY = positionY;
}

void Aircraft::setPositionZ(int positionZ){
    this->positionZ = positionZ;
}

// Setter methods for speed in X, Y, Z directions
void Aircraft::setSpeedX(int speedX){
    this->speedX = speedX;
}

void Aircraft::setSpeedY(int speedY){
    this->speedY = speedY;
}

void Aircraft::setSpeedZ(int positionZ){
    this->speedZ = speedZ;
}

// Start the aircraft operations thread
void Aircraft::startThread(){
    // Create a new thread to perform plane operations
    err_no = pthread_create(&thread, &attr, &Aircraft::planeOperations, this);
    if (err_no != 0){
        printf("ERROR when creating thread number: %d \n", err_no);
    }

    // Uncomment if thread joining is needed in the future
    // err_no = pthread_join(thread, NULL);
    if (err_no != 0){
        printf("ERROR when joining thread number: %d \n", err_no);
    }
}

// Stop the aircraft operations thread
void Aircraft::stopThread(){
    // Cancel the running thread
    err_no = pthread_cancel(thread);
    if (err_no != 0){
        printf("ERROR when cancelling thread number: %d \n", err_no);
    }
}

// Aircraft operations function that updates the aircraft's position periodically
void *Aircraft::planeOperations(void *arg) {
    Aircraft* a = ((Aircraft*)arg);
    cTimer timer(1, 0); // Timer to wait 1 second before updating position

    // Infinite loop to update the aircraft's position
    while(true){
        // Update the position based on speed
        a->setPositionX(a->getPositionX() + a->getSpeedX());
        a->setPositionY(a->getPositionY() + a->getSpeedY());
        a->setPositionZ(a->getPositionZ() + a->getSpeedZ());

        // Wait for the timer to complete before updating again
        timer.waitTimer();
    }

    return NULL;
}

// Function for the aircraft to listen to incoming commands via attach point
void* Aircraft::listen(string attachPoint) {
    name_attach_t *attach;

    // Create a local name attach point for receiving messages
    if ((attach = name_attach(NULL, attachPoint.c_str(), 0)) == NULL) {
        cout << "Aircraft (listen): Error occurred while creating the attach point." << endl;
    }

    AircraftData aircraftCommand;

    // Listen infinitely for incoming commands
    while (true) {
        int rcvid = MsgReceive(attach->chid, &aircraftCommand, sizeof(aircraftCommand), NULL);

        // Handle error if message reception fails
        if (rcvid == -1) {
            break;
        }

        // Handle incoming pulse messages
        if (rcvid == 0) {
            switch (aircraftCommand.header.code) {
            case _PULSE_CODE_DISCONNECT:
                // A client disconnected, detach connection
                ConnectDetach(aircraftCommand.header.scoid);
                continue;
            case _PULSE_CODE_UNBLOCK:
                // REPLY blocked client wants to unblock
                break;
            default:
                // Other pulse code from kernel or other process
                break;
            }
            continue;
        }

        // Handle connect messages and reply with OK
        if (aircraftCommand.header.type == _IO_CONNECT ) {
            MsgReply(rcvid, EOK, NULL, 0);
            continue;
        }

        // Reject unsupported QNX IO messages
        if (aircraftCommand.header.type > _IO_BASE && aircraftCommand.header.type <= _IO_MAX ) {
            MsgError(rcvid, ENOSYS);
            continue;
        }

        // Handle custom message types for position and speed update
        if (aircraftCommand.header.type == 0x00) {
            if (aircraftCommand.header.subtype == 0x01) {
                // Send current aircraft position and velocity (subtype 0x01)
                aircraftCommand.flightId = flightId;
                aircraftCommand.positionX = positionX;
                aircraftCommand.positionY = positionY;
                aircraftCommand.positionZ = positionZ;
                aircraftCommand.speedX = speedX;
                aircraftCommand.speedY = speedY;
                aircraftCommand.speedZ = speedZ;
                MsgReply(rcvid, EOK, &aircraftCommand, sizeof(aircraftCommand));
                break;
            } else if (aircraftCommand.header.subtype == 0x02) {
                // Update current aircraft speed (subtype 0x02)
                speedX = aircraftCommand.speedX;
                speedY = aircraftCommand.speedY;
                speedZ = aircraftCommand.speedZ;

                MsgReply(rcvid, EOK, NULL, 0);
                break;
            } else {
                // Unsupported subtype, return error
                MsgError(rcvid, ENOSYS);
                continue;
            }
        }
    }

    // Detach the name from the name space upon completion
    name_detach(attach, 0);

    return NULL;
}
