#ifndef SRC_COMPUTERSYSTEM_H_
#define SRC_COMPUTERSYSTEM_H_

#include "Aircraft.h"
#include <vector>

using namespace std;

typedef struct {
    int n_seconds;
    int p_interval;
} SeparationData;

class ComputerSystem {
public:
    ComputerSystem();

    static void* MapDisplayThread(void* args);
    static void* dataDisplayThread(void* args);
    static void* computerSystemThread(void* args);
    static void* separationCheckThread(void* args);

    void* MapDisplay();
    void setAircrafts(vector<Aircraft*> aircrafts);
    Aircraft* getAircraftByFlightId(int flightId);
    void computerSystemOperations();
    void separationCheck();
    void alarm();

    virtual ~ComputerSystem();

private:
    vector<Aircraft*> aircrafts;
};

#endif /* SRC_COMPUTERSYSTEM_H_ */
