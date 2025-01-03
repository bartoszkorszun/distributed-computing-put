#ifndef ARBITRATION_H
#define ARBITRATION_H

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

void init_arbitration(int num);
int try_to_get_arbitrator(int rank);
void release_arbitrator(int rank);
void cleanup_arbitration();

#endif
