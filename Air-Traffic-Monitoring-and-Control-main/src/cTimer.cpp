#include "cTimer.h"

cTimer::cTimer(uint32_t sec, uint32_t msec) {
    channel_id = ChannelCreate(0);
    connection_id = ConnectAttach(0, 0, channel_id, 0, 0);
    if (connection_id == -1) {
        std::cerr << "Timer, Connect Attach error: " << errno << "\n";
    }

    SIGEV_PULSE_INIT(&sig_event, connection_id, SIGEV_PULSE_PRIO_INHERIT, 1, 0);

    if (timer_create(CLOCK_REALTIME, &sig_event, &timer_id) == -1) {
        std::cerr << "Timer, Init error: " << errno << "\n";
    }

    setTimerSpec(sec, 1000000 * msec);

    cycles_per_sec = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
}

cTimer::~cTimer() {}

void cTimer::startTimer() {
    timer_settime(timer_id, 0, &timer_spec, NULL);
}

void cTimer::setTimerSpec(uint32_t sec, uint32_t nano) {
    timer_spec.it_value.tv_sec = sec;
    timer_spec.it_value.tv_nsec = nano;
    timer_spec.it_interval.tv_sec = sec;
    timer_spec.it_interval.tv_nsec = nano;
    timer_settime(timer_id, 0, &timer_spec, NULL);
}

void cTimer::waitTimer() {
    MsgReceive(channel_id, &msg_buffer, sizeof(msg_buffer), NULL);
}

void cTimer::tick() {
    tick_cycles = ClockCycles();
}

double cTimer::tock() {
    tock_cycles = ClockCycles();
    return (double)((int)(((double)(tock_cycles - tick_cycles) / cycles_per_sec) * 100000)) / 10;
}
