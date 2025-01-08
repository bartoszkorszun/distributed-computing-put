#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&lamportMutex);
        lamportClock = (lamportClock > pakiet.ts ? lamportClock : pakiet.ts) + 1;
        pthread_mutex_unlock(&lamportMutex);
        
        switch ( status.MPI_TAG ) {
	    case REQUEST: 
		    sendPacket( 0, status.MPI_SOURCE, ACK );
	    break;
	    case ACK: 
	        ackCount++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
	    break;
	    default:
	    break;
        }
    }
}
