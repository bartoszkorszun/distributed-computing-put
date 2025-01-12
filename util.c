#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;
MPI_Datatype MPI_GRUPA_T;

state_t state = InRun;
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;

int lamportClock = 0;
pthread_mutex_t lamportMutex = PTHREAD_MUTEX_INITIALIZER;

group_t myGroup;
pthread_mutex_t groupMutex = PTHREAD_MUTEX_INITIALIZER;

int initiators[MAX_MEMBERS];
int initiatorsCount = 0;
pthread_mutex_t initiatorsMutex = PTHREAD_MUTEX_INITIALIZER;

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

void inicjuj_typ_pakietu()
{
    int blocklengths[NITEMS] = {1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, isInitiator);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

void initGroup() {
    int blocklengths[GITEMS] = {1, MAX_MEMBERS, MAX_MEMBERS};
    MPI_Datatype typy[GITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[GITEMS];
    offsets[0] = offsetof(group_t, groupSize);
    offsets[1] = offsetof(group_t, members);
    offsets[2] = offsetof(group_t, timestamps);

    MPI_Type_create_struct(GITEMS, blocklengths, offsets, typy, &MPI_GRUPA_T);

    MPI_Type_commit(&MPI_GRUPA_T);
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
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    if (freepkt) free(pkt);
}

void sendGroup(group_t *group, int destination, int tag) {
    int freeGroup = 0;
    if (group == 0) { group = malloc(sizeof(group_t)); freeGroup = 1; }

    MPI_Send(group, 1, MPI_GRUPA_T, destination, tag, MPI_COMM_WORLD);
    if (freeGroup) free(group);
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

int addMember(group_t* group, int member, int timestamp) {
    pthread_mutex_lock(&groupMutex);
    // Check if group is full
    if (group->groupSize >= MAX_MEMBERS) {
        pthread_mutex_unlock(&groupMutex);
        return 0;
    }
    
    // Check if member already exists
    for (int i = 0; i < group->groupSize; i++) {
        if (group->members[i] == member) {
            pthread_mutex_unlock(&groupMutex);
            return 0;
        }
    }
    
    group->members[group->groupSize] = member;
    group->timestamps[group->groupSize] = timestamp;
    group->groupSize++;
    println("Member %d added to group", group->members[group->groupSize - 1]);
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