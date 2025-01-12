#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    packet_t pakiet;
    group_t group;
    
    while ( state!=InFinish ) 
    {
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv( &group, 1, MPI_GRUPA_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&lamportMutex);
        lamportClock = (lamportClock > pakiet.ts ? lamportClock : pakiet.ts) + 1;
        pthread_mutex_unlock(&lamportMutex);
        
        switch ( status.MPI_TAG ) 
        {
            case REQUEST: 
                if (state == InWant)
                {
                    sendPacket( &pakiet, status.MPI_SOURCE, ACK );
                    addMember( &myGroup, status.MPI_SOURCE, pakiet.ts );
                    changeState( InGroup );
                    if (pakiet.isInitiator) 
                    {
                        addInitiator( status.MPI_SOURCE );
                    }
                }
                else sendPacket( &pakiet, status.MPI_SOURCE, NACK );
                break;
            case ACK: 
                if (state == InWant || state == InGroup)
                {
                    ackCount++;
                    addMember( &myGroup, status.MPI_SOURCE, pakiet.ts );
                    changeState( InGroup );
                }
                break;
            case NACK:
                if (state == InWant) 
                {
                    nackCount++;
                }
                break;
            case SGRP:
                if (state == InGroup)
                {
                    println("Otrzymałem grupę od %d", status.MPI_SOURCE);
                }
                break;
            default:
                break;
            }
    }
}
