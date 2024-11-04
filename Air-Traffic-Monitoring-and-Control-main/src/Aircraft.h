#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <sys/neutrino.h>

struct AircraftData {
    struct _pulse header;
    int flightId;
    int positionX;
    int positionY;
    int positionZ;
    int speedX;
    int speedY;
    int speedZ;
};

class Aircraft {
private:
    int flightId = 0;
    int positionX = 0;
    int positionY = 0;
    int positionZ = 0;
    int speedX = 0;
    int speedY = 0;
    int speedZ = 0;
    int time = 0; // Consider using std::chrono for more flexible time management

    std::thread thread;
    std::atomic<bool> isRunning{false};

public:
    Aircraft();
    Aircraft(int flightId, int positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time);
    virtual ~Aircraft();

    int getFlightId() const;
    void setFlightId(int flightId);

    int getPositionX() const;
    int getPositionY() const;
    int getPositionZ() const;
    int getPrevPositionX() const;
    int getPrevPositionY() const;
    int getPrevPositionZ() const;
    void setPositionX(int positionX);
    void setPositionY(int positionY);
    void setPositionZ(int positionZ);

    int getSpeedX() const;
    int getSpeedY() const;
    int getSpeedZ() const;
    void setSpeedX(int speedX);
    void setSpeedY(int speedY);
    void setSpeedZ(int speedZ);

    void startThread();
    void stopThread();
    void move();

private:
    void planeOperations();
    void listen(const std::string& attachPoint);
};

struct AircraftListenArgs {
    Aircraft* aircraft;
    std::string attachPoint;
};

#endif /* SRC_AIRCRAFT_H_ */
