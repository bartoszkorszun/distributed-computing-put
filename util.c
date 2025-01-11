#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

state_t state = InRun;
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;

int lamportClock = 0;
pthread_mutex_t lamportMutex = PTHREAD_MUTEX_INITIALIZER;

group_t myGroup;
pthread_mutex_t groupMutex = PTHREAD_MUTEX_INITIALIZER;

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
    int blocklengths[NITEMS] = {1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
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

void initGroup(group_t* group) {
    pthread_mutex_lock(&groupMutex);
    group->size = 0;
    for (int i = 0; i < MAX_MEMBERS; i++) {
        group->members[i] = -1;
    }
    pthread_mutex_unlock(&groupMutex);
}

int addMember(group_t* group, int member) {
    pthread_mutex_lock(&groupMutex);
    
    // Check if group is full
    if (group->size >= MAX_MEMBERS) {
        pthread_mutex_unlock(&groupMutex);
        return 0;
    }
    
    // Check if member already exists
    for (int i = 0; i < group->size; i++) {
        if (group->members[i] == member) {
            pthread_mutex_unlock(&groupMutex);
            return 0;
        }
    }
    
    // Add new member
    group->members[group->size] = member;
    group->size++;
    
    pthread_mutex_unlock(&groupMutex);
    return 1;
}

int isMember(group_t* group, int member) {
    pthread_mutex_lock(&groupMutex);
    
    for (int i = 0; i < group->size; i++) {
        if (group->members[i] == member) {
            pthread_mutex_unlock(&groupMutex);
            return 1;
        }
    }
    
    pthread_mutex_unlock(&groupMutex);
    return 0;
}

int isGroupFull(group_t* group) {
    pthread_mutex_lock(&groupMutex);
    int result = (group->size >= MAX_MEMBERS);
    pthread_mutex_unlock(&groupMutex);
    return result;
}