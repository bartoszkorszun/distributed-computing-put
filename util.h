#ifndef UTILH
#define UTILH
#include "main.h"

typedef struct 
{
    int ts;       
    int src;  
} packet_t;

#define NITEMS 2

#define ACK     1
#define REQUEST 2
#define RELEASE 3
#define APP_PKT 4
#define FINISH  5
#define NACK    6

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum {InRun, InWant, InGroup, InCompetition, InFinish} state_t;
extern state_t state;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t lamportMutex;
extern int lamportClock;

void changeState( state_t );
#endif
