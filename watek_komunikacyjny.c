#include "main.h"
#include "watek_komunikacyjny.h"

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
                    if (packet.isInitiator) 
                    {
                        addInitiator( status.MPI_SOURCE );
                    }
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
                pthread_mutex_lock(&sgrpMutex);
                sgrpCount++;
                pthread_mutex_unlock(&sgrpMutex);

                if (state == InGroup)
                {   
                    pthread_mutex_lock(&groupPacketMutex);
                    for (int i = 0; i < packet.groupSize; i++) 
                    {
                        addMember( packet.members[i], packet.timestamps[i] );
                    }
                    pthread_mutex_unlock(&groupPacketMutex);
                    if (sgrpCount == initiatorsCount) 
                    {
                        for (int i = 0; i < myGroup.groupSize; i++) 
                        {
                            if (rank != myGroup.members[i]) sendGroup( &packet, myGroup.members[i], RGRP );
                        }
                        isGroupFormed = 1;
                    }
                }
            case RGRP:
                pthread_mutex_lock(&rgrpMutex);
                rgrpCount++;
                pthread_mutex_unlock(&rgrpMutex);
                if (state == InGroup)
                {
                    pthread_mutex_lock(&groupPacketMutex);
                    for (int i = 0; i < packet.groupSize; i++) 
                    {
                        addMember( packet.members[i], packet.timestamps[i] );
                    }
                    pthread_mutex_unlock(&groupPacketMutex);
                }
                if (rgrpCount == myGroup.groupSize - 1) 
                {
                    isGroupFormed = 1;
                }
                break;
            case REQ_ARBITERS:
                if (isLeader) 
                {
                    pthread_mutex_lock(&stateMut);
                    if (state == InCompetition) 
                    {
                        sendPacket( &packet, status.MPI_SOURCE, NACK_ARBITERS );
                    }
                    else 
                    {
                        pthread_mutex_lock(&isAskingForArbiterMutex);
                        sendPacket( &packet, status.MPI_SOURCE, ACK_ARBITERS );
                        pthread_mutex_unlock(&isAskingForArbiterMutex);
                    }
                    pthread_mutex_unlock(&stateMut);
                }
                else
                {
                    sendPacket( &packet, status.MPI_SOURCE, ACK_ARBITERS );
                }
                break;
            case ACK_ARBITERS:
                pthread_mutex_lock(&ackArbiterMutex);
                ackArbitersCount++;
                pthread_mutex_unlock(&ackArbiterMutex);
                if (packet.isAskingForArbiter)
                {
                    addOtherLeader( status.MPI_SOURCE, packet.ts );
                }
                if (ackArbitersCount >= size - MAX_ARBITERS) 
                {
                    pthread_mutex_lock(&competitionMutex);
                    if (canStartCompetition() && state == InGroup) 
                    {             
                        printCompetition();
                        changeState( InCompetition );
                        packet_t *pkt = malloc(sizeof(packet_t));
                        for (int i = 0; i < size; i++) 
                        {
                            if (i != rank) sendPacket( pkt, i, START_COMPETITION );

                        }
                        free(pkt);
                    }
                    pthread_mutex_unlock(&competitionMutex);
                }
                break;
            case NACK_ARBITERS:
                pthread_mutex_lock(&nackArbiterMutex);
                nackArbitersCount++;
                pthread_mutex_unlock(&nackArbiterMutex);
                break;
            case START_COMPETITION:
                if (state == InGroup) 
                {
                    for (int i = 0; i < myGroup.groupSize; i++) 
                    {
                        if (myGroup.members[i] == status.MPI_SOURCE) 
                        {
                            changeState( InCompetition );
                            break;
                        }
                    }
                }
                if (isAskingForArbiter)
                {
                    removeOtherLeader( status.MPI_SOURCE );
                }
                break;
            default:
                break;
            }
    }
}
