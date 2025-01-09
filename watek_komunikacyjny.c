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
                int inGroup = 0;
                for ( int i = 0; i < groupCount; i++ ) 
                {
                    for ( int j = 0; j < groups[i].size; j++ ) 
                    {
                        if ( groups[i].members[j] == status.MPI_SOURCE ) 
                        {
                            inGroup = 1;
                            break;
                        }
                    }
                    if ( inGroup ) break;
                }
                if (inGroup) sendPacket( 0, status.MPI_SOURCE, NACK );
                else
                {
                    switch ( state ) 
                    {
                        case InWant: 
                            pthread_mutex_lock(&stateMut);
                            if (groupCount < MAX_GROUPS) 
                            {
                                groups[groupCount].members[groups[groupCount].size++] = status.MPI_SOURCE;
                            }
                            pthread_mutex_unlock(&stateMut);
                            sendPacket( 0, status.MPI_SOURCE, ACK );
                            break;
                        case InGroup: 
                            sendPacket( 0, status.MPI_SOURCE, NACK );
                            break;
                        default: 
                            break;
                    }
                }
                break;
            case ACK: 
                ackCount++; 
                break;
            default:
                break;
            }
    }
}
