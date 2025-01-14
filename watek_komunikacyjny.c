#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    packet_t packet;
    
    while ( state!=InFinish ) 
    {
        MPI_Recv( &packet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&lamportMutex);
        lamportClock = (lamportClock > packet.ts ? lamportClock : packet.ts) + 1;
        pthread_mutex_unlock(&lamportMutex);
        
        switch ( status.MPI_TAG ) 
        {
            case REQUEST: 
                if (state == InWant)
                {
                    sendPacket( &packet, status.MPI_SOURCE, ACK );
                    addMember( status.MPI_SOURCE, packet.ts );
                    changeState( InGroup );
                    if (packet.isInitiator) 
                    {
                        addInitiator( status.MPI_SOURCE );
                    }
                }
                else sendPacket( &packet, status.MPI_SOURCE, NACK );
                break;
            case ACK: 
                if (state == InWant || state == InGroup)
                {
                    ackCount++;
                    addMember( status.MPI_SOURCE, packet.ts );
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
