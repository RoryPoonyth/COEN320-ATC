#ifndef SRC_COMMUNICATIONSYSTEM_H_
#define SRC_COMMUNICATIONSYSTEM_H_

#include "Aircraft.h"

using namespace std;

class CommunicationSystem {
public:
    CommunicationSystem();

    void* relayNewSpeed(Aircraft &aircraft, int newSpeedX, int newSpeedY, int newSpeedZ);
    void* relayNewPosition(Aircraft &aircraft, int newPositionX, int newPositionY, int newPositionZ);

    virtual ~CommunicationSystem();
};

#endif /* SRC_COMMUNICATIONSYSTEM_H_ */
