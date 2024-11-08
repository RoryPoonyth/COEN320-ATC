#include "cTimer.h"

// Constructor to initialize the timer with specified seconds and milliseconds
cTimer::cTimer(uint32_t sec, uint32_t msec) {
	// Create a communication channel for the timer
	channel_id = ChannelCreate(0);
	connection_id = ConnectAttach(0,0,channel_id,0,0);
	if(connection_id == -1){
		std::cerr << "Timer, Connect Attach error : " << errno << "\n"; // Error if connection fails
	}

	// Initialize the signal event structure for the timer
	SIGEV_PULSE_INIT(&sig_event, connection_id, SIGEV_PULSE_PRIO_INHERIT, 1, 0);

	// Create a timer associated with the signal event
	if (timer_create(CLOCK_REALTIME, &sig_event, &timer_id) == -1){
		std::cerr << "Timer, Init error : " << errno << "\n"; // Error if timer creation fails
	}

	// Set the timer specification with the given seconds and milliseconds
	setTimerSpec(sec,1000000 * msec);

	// Retrieve system cycles per second for time measurement
	cycles_per_sec = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
}

// Destructor for cTimer, cleans up resources
cTimer::~cTimer() {
    // Detach the connection if it was successfully created
    if (connection_id != -1) {
        ConnectDetach(connection_id);
    }

    // Destroy the communication channel if it was successfully created
    if (channel_id != -1) {
        ChannelDestroy(channel_id);
    }

    // Delete the timer if it was successfully created
    timer_delete(timer_id);
}

// Starts the timer with the specified timing configuration
void cTimer::startTimer(){
	timer_settime(timer_id, 0, &timer_spec, NULL);
}

// Configures the timer specification with seconds and nanoseconds intervals
void cTimer::setTimerSpec(uint32_t sec, uint32_t nano){
	timer_spec.it_value.tv_sec = sec; // Initial timer duration in seconds
	timer_spec.it_value.tv_nsec = nano; // Initial timer duration in nanoseconds
	timer_spec.it_interval.tv_sec = sec; // Interval duration in seconds
	timer_spec.it_interval.tv_nsec = nano; // Interval duration in nanoseconds
	timer_settime(timer_id, 0, &timer_spec, NULL); // Apply the timer configuration
}

// Waits for a timer signal/message on the communication channel
void cTimer::waitTimer() {
    int rcvid = MsgReceive(channel_id, &msg_buffer, sizeof(msg_buffer), NULL);

    if (rcvid == -1) {
        std::cerr << "Error receiving message: " << errno << "\n"; // Error if message receive fails
        // Error handling code here
    } else {
        // Optional: handle the received message
    }
}

// Marks the start time in CPU cycles
void cTimer::tick(){
	tick_cycles = ClockCycles();
}

// Marks the end time in CPU cycles and calculates the elapsed time in milliseconds
double cTimer::tock(){
	tock_cycles = ClockCycles();
	return (double)((int)(((double)(tock_cycles - tick_cycles) / cycles_per_sec) * 100000)) / 10;
}
