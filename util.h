#ifndef UTILH
#define UTILH
#include "main.h"

typedef struct 
{
    int ts;       
    int src;  
    int isInitiator;
} packet_t;

#define NITEMS 3

#define ACK     1
#define REQUEST 2
#define RELEASE 3
#define APP_PKT 4
#define FINISH  5
#define NACK    6
#define SGRP    7
#define RGRP    8

extern MPI_Datatype MPI_PAKIET_T;
void init_packet_type();

void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum {InRun, InWant, InGroup, InCompetition, InFinish} state_t;
extern state_t state;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t lamportMutex;
extern int lamportClock;

void changeState( state_t );

extern int groupSize;
#define MAX_MEMBERS 10

typedef struct 
{
    int members[MAX_MEMBERS];
    int timestamps[MAX_MEMBERS];
    int groupSize;
} group_t;
void initGroup(void);
extern group_t myGroup;
extern pthread_mutex_t groupMutex;
void sendGroup(int destination, int tag);
int addMember(int member, int timestamp);

extern int initiators[MAX_MEMBERS];
extern int initiatorsCount;
extern pthread_mutex_t initiatorsMutex;
int addInitiator(int initiator);

#endif
