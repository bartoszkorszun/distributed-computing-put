#ifndef LAMPORT_H
#define LAMPORT_H

#include <pthread.h>

void init_lamport_clock();
void increment_clock();
void update_clock(int received_clock);
int get_clock();
void cleanup_lamport_clock();

#endif
