#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    packet_t pakiet;
    
    while ( state!=InFinish ) 
    {
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&lamportMutex);
        lamportClock = (lamportClock > pakiet.ts ? lamportClock : pakiet.ts) + 1;
        pthread_mutex_unlock(&lamportMutex);
        
        switch ( status.MPI_TAG ) 
        {
            case REQUEST: 
                if (state == InWant)
                {
                    sendPacket( &pakiet, status.MPI_SOURCE, ACK );
                    addMember( &myGroup, status.MPI_SOURCE );
                    changeState( InGroup );
                }
                else sendPacket( &pakiet, status.MPI_SOURCE, NACK );
                break;
            case ACK: 
                if (state == InWant || state == InGroup)
                {
                    ackCount++;
                    addMember( &myGroup, status.MPI_SOURCE );
                    changeState( InGroup );
                }
                break;
            case NACK:
                if (state == InWant) 
                {
                    println("Otrzymałem NACK od %d", status.MPI_SOURCE);
                }
                break;
            default:
                break;
            }
    }
}
