#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

int isInitiator = 1;

state_t state = InRun;
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;

int lamportClock = 0;
pthread_mutex_t lamportMutex = PTHREAD_MUTEX_INITIALIZER;

group_t myGroup;
pthread_mutex_t groupMutex = PTHREAD_MUTEX_INITIALIZER;

int initiators[MAX_MEMBERS];
int initiatorsCount = 0;
pthread_mutex_t initiatorsMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t groupPacketMutex = PTHREAD_MUTEX_INITIALIZER;

int sgrpCount = 0;
int rgrpCount = 0;
pthread_mutex_t sgrpMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rgrpMutex = PTHREAD_MUTEX_INITIALIZER;

int isGroupFormed = 0;

struct tagNames_t
{
    const char *name;
    int tag;
} 
tagNames[] = 
{
    {
        "pakiet aplikacyjny", APP_PKT 
    }, 
    {
        "finish", FINISH
    }, 
    {
        "potwierdzenie", ACK
    }, 
    {
        "prośba o dołączenie do grupy", REQUEST
    }, 
    {
        "rozwiązanie grupy", RELEASE
    }, 
    {
        "odmowa", NACK
    }
};

const char *const tag2string( int tag )
{
    for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) 
    {
	    if ( tagNames[i].tag == tag )  return tagNames[i].name;
    }
    return "<unknown>";
}

void init_packet_type()
{
    int blocklengths[NITEMS] = {1,1,1,MAX_MEMBERS,MAX_MEMBERS,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, isInitiator);
    offsets[3] = offsetof(packet_t, members);
    offsets[4] = offsetof(packet_t, timestamps);
    offsets[5] = offsetof(packet_t, groupSize);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

void initGroup(void) {
    pthread_mutex_lock(&groupMutex);
    myGroup.groupSize = 0;
    for (int i = 0; i < MAX_MEMBERS; i++) {
        myGroup.members[i] = -1;
        myGroup.timestamps[i] = -1;
    }
    pthread_mutex_unlock(&groupMutex);
}

void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt = 0;
    if (pkt == 0) { pkt = malloc(sizeof(packet_t)); freepkt = 1; }

    pthread_mutex_lock(&lamportMutex);
    lamportClock++;
    pkt->ts = lamportClock;
    pthread_mutex_unlock(&lamportMutex);

    pkt->src = rank;

    if (isInitiator) pkt->isInitiator = 1;
    else pkt->isInitiator = 0;

    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(pkt);
}

void sendGroup(packet_t *gpkt, int destination, int tag) {
    int freepkt = 0;
    if (gpkt == 0) { gpkt = malloc(sizeof(packet_t)); freepkt = 1; }

    for (int i = 0; i < myGroup.groupSize; i++) {
        gpkt->members[i] = myGroup.members[i];
        gpkt->timestamps[i] = myGroup.timestamps[i];
    }
    gpkt->groupSize = myGroup.groupSize;

    MPI_Send(gpkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(gpkt);
}

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

int addMember(int member, int timestamp) {
    pthread_mutex_lock(&groupMutex);
    // Check if group is full
    if (myGroup.groupSize >= MAX_MEMBERS) {
        pthread_mutex_unlock(&groupMutex);
        return 0;
    }
    
    // Check if member already exists
    for (int i = 0; i < myGroup.groupSize; i++) {
        if (myGroup.members[i] == member) {
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

int addInitiator(int initiator) {
    pthread_mutex_lock(&initiatorsMutex);

    if (initiatorsCount >= MAX_MEMBERS) {
        pthread_mutex_unlock(&initiatorsMutex);
        return 0;
    }

    for (int i = 0; i < initiatorsCount; i++) {
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