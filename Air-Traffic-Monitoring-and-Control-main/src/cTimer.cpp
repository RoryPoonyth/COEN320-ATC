#include "cTimer.h"
#include <iostream>
#include <sys/neutrino.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/syspage.h>

cTimer::cTimer(uint32_t sec, uint32_t msec) {
    // Create a channel for the timer
    channel_id = ChannelCreate(0);
    if (channel_id == -1) {
        perror("ChannelCreate failed");
        return;
    }

    // Connect to the channel for timer notifications
    connection_id = ConnectAttach(0, 0, channel_id, _NTO_SIDE_CHANNEL, 0);
    if (connection_id == -1) {
        perror("ConnectAttach failed");
        ChannelDestroy(channel_id);
        return;
    }

    // Set up the event to be sent to our connection when the timer expires
    SIGEV_PULSE_INIT(&sig_event, connection_id, SIGEV_PULSE_PRIO_INHERIT, 1, 0);

    // Create the timer
    if (timer_create(CLOCK_REALTIME, &sig_event, &timer_id) == -1) {
        perror("timer_create failed");
        ConnectDetach(connection_id);
        ChannelDestroy(channel_id);
        return;
    }

    // Set timer specifications (initial interval)
    setTimerSpec(sec, msec * 1000000);
}

cTimer::~cTimer() {
    timer_delete(timer_id);
    ConnectDetach(connection_id);
    ChannelDestroy(channel_id);
}

void cTimer::setTimerSpec(uint32_t sec, uint32_t nano) {
    timer_spec.it_value.tv_sec = sec;
    timer_spec.it_value.tv_nsec = nano;
    timer_spec.it_interval.tv_sec = sec;
    timer_spec.it_interval.tv_nsec = nano;
}

void cTimer::startTimer() {
    if (timer_settime(timer_id, 0, &timer_spec, nullptr) == -1) {
        perror("timer_settime failed");
    }
}

void cTimer::waitTimer() {
    struct _pulse pulse;
    int rcvid = MsgReceive(channel_id, &pulse, sizeof(pulse), nullptr);
    if (rcvid == -1) {
        perror("MsgReceive failed");
    }
}

void cTimer::tick() {
    tick_cycles = ClockCycles();
}

double cTimer::tock() {
    tock_cycles = ClockCycles();
    uint64_t elapsed_cycles = tock_cycles - tick_cycles;
    return static_cast<double>(elapsed_cycles) / static_cast<double>(SYSPAGE_ENTRY(qtime)->cycles_per_sec) * 1000.0;
}
