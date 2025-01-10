#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "util.h"

#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 1
#define STATE_CHANGE_PROB 10
#define MAX_ARBITERS 5
#define MAX_MEMBERS 10

#define ROOT 0

extern int rank;
extern int size;
extern int ackCount;
extern int availableArbiters;
extern pthread_mutex_t arbiterMutex;
extern pthread_t threadKom;

typedef struct 
{
    int members[MAX_MEMBERS];
    int size;
} group_t;

// makro println
#define println(FORMAT,...) printf("%c[%d;%dm [%d] [LC:%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamportClock, ##__VA_ARGS__, 27,0,37);
#endif
