#ifndef CTIMER_H_
#define CTIMER_H_

#include <stdio.h>
#include <iostream>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sync.h>
#include <sys/siginfo.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/syspage.h>
#include <inttypes.h>
#include <stdint.h>

// cTimer class: Manages timer functionality using POSIX timers and QNX features
class cTimer {
	int channel_id; // Channel ID for inter-process communication
	int connection_id; // Connection ID for attaching to the channel

	struct sigevent sig_event; // Signal event structure for the timer
	struct itimerspec timer_spec; // Timer specification (interval and initial delay)
	timer_t timer_id; // Timer ID

	char msg_buffer[100]; // Buffer for storing messages received by the timer

	uint64_t cycles_per_sec; // System clock cycles per second
	uint64_t tick_cycles, tock_cycles; // Variables to store tick and tock cycles for elapsed time

public:
	// Constructor: Initializes the timer with specified seconds and milliseconds
	cTimer(uint32_t sec, uint32_t msec);

	// Sets the timer specification with the specified seconds and nanoseconds
	void setTimerSpec(uint32_t sec, uint32_t nano);

	// Waits for the timer to trigger and receive a message on the channel
	void waitTimer();

	// Starts the timer using the configured specification
	void startTimer();

	// Records the start time in CPU cycles for timing measurements
	void tick();

	// Records the end time in CPU cycles and returns the elapsed time in milliseconds
	double tock();

	// Destructor: Cleans up resources, including channel and timer deletion
	virtual ~cTimer();
};

#endif /* CTIMER_H_ */
