#ifndef UTILH
#define UTILH
#include "main.h"

// CONSTANTS
#define MAX_ARBITERS 3
#define MAX_MEMBERS 32

// TAGS
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
#define END_COMPETITION 13

// PACKET STRUCTURE
extern MPI_Datatype MPI_PAKIET_T;
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
void init_packet_type();
void sendPacket(packet_t *pkt, int destination, int tag);

// GROUP STRUCTURE
typedef struct 
{
    int members[MAX_MEMBERS];
    int timestamps[MAX_MEMBERS];
    int groupSize;
} group_t;
extern int groupSize;
extern group_t myGroup;
extern pthread_mutex_t groupMutex;
extern pthread_mutex_t groupPacketMutex;
void initGroup(void);
void sendGroup(packet_t *gpkt, int destination, int tag);
int addMember(int member, int timestamp);

// LEADERS STRUCTURE
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

// STATES
typedef enum {InRun, InWant, InGroup, InCompetition, InFinish} state_t;
extern state_t state;
extern pthread_mutex_t stateMut;
void changeState( state_t );

// UTILS
extern int ackArbitersCount;
extern int ackCount;
extern int isAskingForArbiter;
extern int isGroupFormed;
extern int isInitiator;
extern int isLeader;
extern int lamportClock;
extern int nackArbitersCount;
extern int nackCount;
extern int rgrpCount;
extern int sgrpCount;

extern pthread_mutex_t ackArbiterMutex;
extern pthread_mutex_t isAskingForArbiterMutex;
extern pthread_mutex_t lamportMutex;
extern pthread_mutex_t nackArbiterMutex;
extern pthread_mutex_t rgrpMutex;
extern pthread_mutex_t sgrpMutex;

// INITIATORS
extern int initiators[MAX_MEMBERS];
extern int initiatorsCount;
extern pthread_mutex_t initiatorsMutex;
int addInitiator(int initiator);
void resetInitiators(void);

void chooseLeader();

extern pthread_mutex_t competitionMutex;
int canStartCompetition();
void printCompetition();

void resetValues();
#endif
