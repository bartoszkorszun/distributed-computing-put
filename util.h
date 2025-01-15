#ifndef UTILH
#define UTILH
#include "main.h"

#define MAX_ARBITERS 3
#define MAX_MEMBERS 32

typedef struct 
{
    int ts;       
    int src;  
    int isInitiator;
    int isAskingForArbiter;
    int members[MAX_MEMBERS];
    int timestamps[MAX_MEMBERS];
    int groupSize;
} packet_t;

#define NITEMS 7

#define ACK     1
#define REQUEST 2
#define RELEASE 3
#define APP_PKT 4
#define FINISH  5
#define NACK    6
#define SGRP    7
#define RGRP    8
#define REQ_ARBITERS 9
#define ACK_ARBITERS 10
#define NACK_ARBITERS 11
#define START_COMPETITION 12

extern MPI_Datatype MPI_PAKIET_T;
void init_packet_type();

void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum {InRun, InWant, InGroup, InCompetition, InFinish} state_t;
extern state_t state;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t lamportMutex;
extern pthread_mutex_t groupPacketMutex;
extern int sgrpCount;
extern int rgrpCount;
extern int isInitiator;
extern int lamportClock;
extern int isGroupFormed;
extern int isLeader;
extern int isAskingForArbiter;
extern pthread_mutex_t isAskingForArbiterMutex;

extern int ackArbitersCount;
extern int nackArbitersCount;
extern pthread_mutex_t ackArbiterMutex;
extern pthread_mutex_t nackArbiterMutex;

extern pthread_mutex_t sgrpMutex;
extern pthread_mutex_t rgrpMutex;

void changeState( state_t );

extern int groupSize;

typedef struct 
{
    int members[MAX_MEMBERS];
    int timestamps[MAX_MEMBERS];
    int groupSize;
} group_t;

typedef struct 
{
    int leaders[MAX_MEMBERS];
    int timestamps[MAX_MEMBERS];
    int count;
} leaders_t;

extern leaders_t otherLeaders;
extern pthread_mutex_t otherLeadersMutex;
void initOtherLeaders(void);
void addOtherLeader(int leader, int timestamp);
void removeOtherLeader(int leader);
void initGroup(void);
extern group_t myGroup;
extern pthread_mutex_t groupMutex;
void sendGroup(packet_t *gpkt, int destination, int tag);
int addMember(int member, int timestamp);
extern int initiators[MAX_MEMBERS];
extern int initiatorsCount;
extern pthread_mutex_t initiatorsMutex;
int addInitiator(int initiator);

extern pthread_mutex_t competitionMutex;

int canStartCompetition();

void chooseLeader();

void printCompetition();

#endif
