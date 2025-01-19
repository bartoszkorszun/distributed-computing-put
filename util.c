#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

int ackArbitersCount = 0;
int ackCount = 0;
int initiatorsCount = 0;
int isAskingForArbiter = 0;
int isGroupFormed = 0;
int isInitiator = 1;
int isLeader = 0;
int lamportClock = 0;
int nackArbitersCount = 0;
int nackCount = 0;
int sgrpCount = 0;

pthread_mutex_t ackArbiterMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ackMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t isAskingForArbiterMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t isGroupFormedMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t isInitiatorMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t isLeaderMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lamportMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t nackArbiterMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t nackMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sgrpMutex = PTHREAD_MUTEX_INITIALIZER;

int initiators[MAX_MEMBERS];

state_t state = InRun;

group_t myGroup;
leaders_t otherLeaders;

pthread_mutex_t competitionMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t groupMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t groupPacketMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t initiatorsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t otherLeadersMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;


struct tagNames_t
{
    const char *name;
    int tag;
} 
tagNames[] = 
{
    { "potwierdzenie", ACK }, 
    { "prośba o dołączenie do grupy", REQUEST }, 
    { "rozwiązanie grupy", RELEASE }, 
    { "pakiet aplikacyjny", APP_PKT }, 
    { "finish", FINISH }, 
    { "odmowa", NACK },
    { "wymiana grupy między inicjatorami", SGRP },
    { "wymiana grupy między członkami", RGRP },
    { "prośba o arbitra", REQ_ARBITERS },
    { "potwierdzenie arbitra", ACK_ARBITERS },
    { "odmowa arbitra", NACK_ARBITERS },
    { "start zawodów", START_COMPETITION },
    { "koniec zawodów", END_COMPETITION }
};

const char *const tag2string( int tag )
{
    for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) 
    {
	    if ( tagNames[i].tag == tag )  return tagNames[i].name;
    }
    return "<unknown>";
}

// INITS
void init_packet_type()
{
    int blocklengths[NITEMS] = {1,1,1,1,MAX_MEMBERS,MAX_MEMBERS,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, isInitiator);
    offsets[3] = offsetof(packet_t, isAskingForArbiter);
    offsets[4] = offsetof(packet_t, members);
    offsets[5] = offsetof(packet_t, timestamps);
    offsets[6] = offsetof(packet_t, groupSize);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

void initGroup(void) 
{
    pthread_mutex_lock(&groupMutex);
    myGroup.groupSize = 0;
    for (int i = 0; i < MAX_MEMBERS; i++) 
    {
        myGroup.members[i] = -1;
        myGroup.timestamps[i] = -1;
    }
    pthread_mutex_unlock(&groupMutex);
}

void initOtherLeaders(void) 
{
    pthread_mutex_lock(&otherLeadersMutex);
    otherLeaders.count = 0;
    for (int i = 0; i < MAX_MEMBERS; i++) 
    {
        otherLeaders.leaders[i] = -1;
        otherLeaders.timestamps[i] = -1;
    }
    pthread_mutex_unlock(&otherLeadersMutex);
}

// GROUPS MANAGEMENT
int addMember(int member, int timestamp) 
{
    pthread_mutex_lock(&groupMutex);
    if (myGroup.groupSize >= MAX_MEMBERS) 
    {
        pthread_mutex_unlock(&groupMutex);
        return 0;
    }
    
    for (int i = 0; i < myGroup.groupSize; i++) 
    {
        if (myGroup.members[i] == member) 
        {
            if (myGroup.timestamps[i] < timestamp) 
            {
                myGroup.timestamps[i] = timestamp;
            }
            pthread_mutex_unlock(&groupMutex);
            return 0;
        }
    }
    
    myGroup.members[myGroup.groupSize] = member;
    myGroup.timestamps[myGroup.groupSize] = timestamp;
    myGroup.groupSize++;
    pthread_mutex_unlock(&groupMutex);
    return 1;
}

int addInitiator(int initiator) 
{
    pthread_mutex_lock(&initiatorsMutex);

    if (initiatorsCount >= MAX_MEMBERS) 
    {
        pthread_mutex_unlock(&initiatorsMutex);
        return 0;
    }

    for (int i = 0; i < initiatorsCount; i++) 
    {
        if (initiators[i] == initiator) {
            pthread_mutex_unlock(&initiatorsMutex);
            return 0;
        }
    }

    initiators[initiatorsCount] = initiator;
    initiatorsCount++;
    pthread_mutex_unlock(&initiatorsMutex);
    return 1;
}

void resetInitiators() 
{
    pthread_mutex_lock(&initiatorsMutex);
    initiatorsCount = 0;
    for (int i = 0; i < MAX_MEMBERS; i++) 
    {
        initiators[i] = -1;
    }
    pthread_mutex_unlock(&initiatorsMutex);
}

void addOtherLeader(int leader, int timestamp) 
{
    pthread_mutex_lock(&otherLeadersMutex);
    for (int i = 0; i < MAX_MEMBERS; i++) 
    {
        if (otherLeaders.leaders[i] == leader) 
        {
            pthread_mutex_unlock(&otherLeadersMutex);
            return;
        }
    }
    otherLeaders.leaders[otherLeaders.count] = leader;
    otherLeaders.timestamps[otherLeaders.count] = timestamp;
    otherLeaders.count++;
    pthread_mutex_unlock(&otherLeadersMutex);
}

void removeOtherLeader(int leader) 
{
    pthread_mutex_lock(&otherLeadersMutex);
    for (int i = 0; i < otherLeaders.count; i++) 
    {
        if (otherLeaders.leaders[i] == leader) 
        {
            for (int j = i; j < otherLeaders.count - 1; j++) 
            {
                otherLeaders.leaders[j] = otherLeaders.leaders[j + 1];
                otherLeaders.timestamps[j] = otherLeaders.timestamps[j + 1];
            }
            otherLeaders.leaders[otherLeaders.count - 1] = -1;
            otherLeaders.timestamps[otherLeaders.count - 1] = -1;
            otherLeaders.count--;
            pthread_mutex_unlock(&otherLeadersMutex);
            return;
        }
    }
    pthread_mutex_unlock(&otherLeadersMutex);
}

// SENDS
void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt = 0;
    if (pkt == 0) { pkt = malloc(sizeof(packet_t)); freepkt = 1; }

    pthread_mutex_lock(&lamportMutex);
    lamportClock++;
    pkt->ts = lamportClock;
    pthread_mutex_unlock(&lamportMutex);

    pkt->src = rank;

    pthread_mutex_lock(&isInitiatorMutex);
    if (state == InWant) pkt->isInitiator = isInitiator;
    else pkt->isInitiator = 0;
    pthread_mutex_unlock(&isInitiatorMutex);

    pkt->isAskingForArbiter = isAskingForArbiter;

    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(pkt);
}

void sendGroup(packet_t *gpkt, int destination, int tag) 
{
    int freepkt = 0;
    if (gpkt == 0) { gpkt = malloc(sizeof(packet_t)); freepkt = 1; }

    gpkt->src = rank;
    gpkt->isInitiator = 0;
    pthread_mutex_lock(&lamportMutex);
    gpkt->ts = lamportClock;
    pthread_mutex_unlock(&lamportMutex);

    for (int i = 0; i < myGroup.groupSize; i++) 
    {
        gpkt->members[i] = myGroup.members[i];
        gpkt->timestamps[i] = myGroup.timestamps[i];
    }
    gpkt->groupSize = myGroup.groupSize;

    MPI_Send(gpkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(gpkt);
}

// UTILS
void changeState( state_t newState )
{
    pthread_mutex_lock( &stateMut );
    if (state == InFinish) 
    { 
	    pthread_mutex_unlock( &stateMut );
        return;
    }
    
    state = newState;
    pthread_mutex_unlock( &stateMut );
}

void chooseLeader() {
    int leader = -1;
    int leaderTS = __INT_MAX__;
    pthread_mutex_lock(&groupMutex);
    for (int i = 0; i < myGroup.groupSize; i++) 
    {
        if (myGroup.timestamps[i] < leaderTS) 
        {
            leaderTS = myGroup.timestamps[i];
            leader = myGroup.members[i];
        }
    }
    pthread_mutex_unlock(&groupMutex);
    if (leader == rank) 
    {
        isLeader = 1;
    }
}

int canStartCompetition() {
    pthread_mutex_lock(&otherLeadersMutex);
    int minTS = __INT_MAX__;
    int leader = -1;
    for (int i = 0; i < otherLeaders.count; i++) 
    {
        if (otherLeaders.timestamps[i] < minTS) 
        {
            minTS = otherLeaders.timestamps[i];
            leader = otherLeaders.leaders[i];
        }
    }
    if (leader == rank) 
    {
        pthread_mutex_unlock(&otherLeadersMutex);
        return 1;
    } 
    pthread_mutex_unlock(&otherLeadersMutex);
    return 0;
}

void printCompetition() {
    println("ackCount: %d, size %d, arbiters: %d", ackArbitersCount, size, MAX_ARBITERS);
    println("Zaczynam zawody\nLeader: %d\nUczestnicy:", rank);
    for (int i = 0; i < myGroup.groupSize; i++) 
    {
        printf(" - %d\n", myGroup.members[i]);
    }
}

void resetValues() 
{
    pthread_mutex_lock(&ackMutex);
    ackCount = 0;
    pthread_mutex_unlock(&ackMutex);

    pthread_mutex_lock(&nackMutex);
    nackCount = 0;
    pthread_mutex_unlock(&nackMutex);

    pthread_mutex_lock(&ackArbiterMutex);
    ackArbitersCount = 0;
    pthread_mutex_unlock(&ackArbiterMutex);

    pthread_mutex_lock(&nackArbiterMutex);
    nackArbitersCount = 0;
    pthread_mutex_unlock(&nackArbiterMutex);

    pthread_mutex_lock(&isAskingForArbiterMutex);
    isAskingForArbiter = 0;
    pthread_mutex_unlock(&isAskingForArbiterMutex);

    pthread_mutex_lock(&isGroupFormedMutex);
    isGroupFormed = 0;
    pthread_mutex_unlock(&isGroupFormedMutex);

    pthread_mutex_lock(&isInitiatorMutex);
    isInitiator = 1;
    pthread_mutex_unlock(&isInitiatorMutex);

    pthread_mutex_lock(&isLeaderMutex);
    isLeader = 0;
    pthread_mutex_unlock(&isLeaderMutex);

    pthread_mutex_lock(&sgrpMutex);
    sgrpCount = 0;
    pthread_mutex_unlock(&sgrpMutex);

    resetInitiators();
    initGroup();
    initOtherLeaders();
}