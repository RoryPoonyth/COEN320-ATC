#ifndef CTIMER_H_
#define CTIMER_H_

#include <iostream>
#include <ctime>
#include <cerrno>
#include <cstdint>
#include <sys/siginfo.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>

class cTimer {
private:
    int channel_id;
    int connection_id;

    struct sigevent sig_event;
    struct itimerspec timer_spec;
    timer_t timer_id;

    char msg_buffer[100];

    uint64_t cycles_per_sec;
    uint64_t tick_cycles, tock_cycles;

public:
    // Constructor to initialize timer with given seconds and milliseconds
    cTimer(uint32_t sec, uint32_t msec);

    // Destructor to clean up resources
    virtual ~cTimer();

    // Set the timer specifications
    void setTimerSpec(uint32_t sec, uint32_t nano);

    // Start the timer
    void startTimer();

    // Wait for the timer to expire
    void waitTimer();

    // Record the start time
    void tick();

    // Calculate and return the elapsed time in milliseconds
    double tock();
};

#endif /* CTIMER_H_ */
