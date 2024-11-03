#include <pthread.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/neutrino.h>
#ifndef SRC_AIRCRAFT_H_
#define SRC_AIRCRAFT_H_

using namespace std;

typedef struct {
	struct _pulse header;
	int flightId;
	int positionX;
	int positionY;
	int positionZ;

	int speedX;
	int speedY;
	int speedZ;
} AircraftData;

class Aircraft {
private:
	int flightId = 0;
	int positionX = 0;
	int positionY = 0;
	int positionZ = 0;
	int speedX = 0;
	int speedY = 0;
	int speedZ = 0;
	int time = 0; // We can discuss which type this variable should be
	int err_no = 0;
	pthread_attr_t attr;
	pthread_t thread;

public:
	Aircraft();
	Aircraft(int flightId, int positionX, int positionY, int positionZ, int speedX, int speedY, int speedZ, int time);
	virtual ~Aircraft();

	int getFlightId();
	void setFlightId(int flightId);

	int getPositionX();
	int getPositionY();
	int getPositionZ();
	int getPrevPositionX();
	int getPrevPositionY();
	int getPrevPositionZ();
	void setPositionX(int positionX);
	void setPositionY(int positionY);
	void setPositionZ(int positionZ);

	int getSpeedX();
	int getSpeedY();
	int getSpeedZ();
	void setSpeedX(int speedX);
	void setSpeedY(int speedY);
	void setSpeedZ(int speedZ);

	void startThread();
	void stopThread();

	static void *planeOperations(void *arg);
	static void* aircraftListenHelper(void* args);

	void move();
	void* listen(string attachPoint);
};

struct AircraftListenArgs {
	Aircraft* aircraft;
	string attachPoint;
};

#endif /* SRC_AIRCRAFT_H_ */
