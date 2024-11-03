#ifndef SRC_OPERATORCONSOLE_H_
#define SRC_OPERATORCONSOLE_H_

#include <string>
#include <vector>
#include <iostream>
#include <sys/dispatch.h>

using namespace std;

class OperatorConsole {
private:
    string log_entry = "";

public:
    OperatorConsole();

    static void* listenForOperatorInput(void* args);
    static void* displayAircraftDataThread(void* args);
    static void* updateAircraftSpeedThread(void* args);

    void listenForUserInput();
    void* updateAircraftSpeed();
    void* displayAircraftData();

    void writeLog(string log_entry);

    virtual ~OperatorConsole();
};

#endif /* SRC_OPERATORCONSOLE_H_ */
