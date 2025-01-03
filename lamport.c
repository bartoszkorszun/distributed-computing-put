#include "lamport.h"

int lamport_clock = 0;
pthread_mutex_t clock_mutex;

void init_lamport_clock() {
    pthread_mutex_init(&clock_mutex, NULL);
}

void increment_clock() {
    pthread_mutex_lock(&clock_mutex);
    lamport_clock++;
    pthread_mutex_unlock(&clock_mutex);
}

void update_clock(int received_clock) {
    pthread_mutex_lock(&clock_mutex);
    if (received_clock > lamport_clock) {
        lamport_clock = received_clock;
    }
    lamport_clock++;
    pthread_mutex_unlock(&clock_mutex);
}

int get_clock() {
    pthread_mutex_lock(&clock_mutex);
    int clock = lamport_clock;
    pthread_mutex_unlock(&clock_mutex);
    return clock;
}

void cleanup_lamport_clock() {
    pthread_mutex_destroy(&clock_mutex);
}
