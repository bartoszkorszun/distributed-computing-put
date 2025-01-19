#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;
	int hasAskedForArbiter = 0;
	int hasSentGroup = 0;
	int inCompetition = 0;

    while (state != InFinish) 
	{
		switch (state) 
		{
			case InRun: 
				perc = random()%100;
				if ( perc < 15 ) 
				{
					resetValues();
					hasAskedForArbiter = 0;
					hasSentGroup = 0;
					inCompetition = 0;
					println("Chcę się napić");
					packet_t *pkt = malloc(sizeof(packet_t));
					changeState( InWant ); 
					for (int i = 0; i <= size-1; i++)
						if (i != rank) sendPacket( pkt, i, REQUEST );
					free(pkt);
					pthread_mutex_lock(&lamportMutex);
					addMember(rank, lamportClock);
					pthread_mutex_unlock(&lamportMutex);
				} 
				break;
			case InWant:
				println("Czekam na utworzenie grupy");
				break;
			case InGroup:
				pthread_mutex_lock(&isInitiatorMutex);
				if (isInitiator && !hasSentGroup)
				{
					hasSentGroup = 1;
					pthread_mutex_lock(&groupPacketMutex);
					packet_t *gpkt = malloc(sizeof(packet_t));
					for (int i = 0; i < initiatorsCount; i++) 
					{
						sendGroup( gpkt, initiators[i], SGRP );
					}
					pthread_mutex_unlock(&groupPacketMutex);
					free(gpkt);
				}	
				pthread_mutex_unlock(&isInitiatorMutex);

				pthread_mutex_lock(&isGroupFormedMutex);
				if (isGroupFormed) 
				{
					chooseLeader();
					pthread_mutex_lock(&isLeaderMutex);
					if (isLeader && !hasAskedForArbiter) 
					{
						println("Jestem liderem");
						packet_t *pkt = malloc(sizeof(packet_t));
						pthread_mutex_lock(&isAskingForArbiterMutex);
						for (int i = 0; i < size; i++) 
						{
							if (i != rank) sendPacket( pkt, i, REQ_ARBITERS );
						}
						isAskingForArbiter = 1;
						pthread_mutex_lock(&lamportMutex);
						addOtherLeader(rank, lamportClock);
						pthread_mutex_unlock(&lamportMutex);
						pthread_mutex_unlock(&isAskingForArbiterMutex);
						free(pkt);
						hasAskedForArbiter = 1;
					}
					if(isLeader) { println("czekam na wolnych arbitrów") }
					pthread_mutex_unlock(&isLeaderMutex);
				}
				pthread_mutex_unlock(&isGroupFormedMutex);
				break;
			case InCompetition:
				println("W trakcie zawodów: %d", 10 - inCompetition);
				pthread_mutex_lock(&competitionMutex);
				sleep(SEC_IN_STATE);
				inCompetition++;
				pthread_mutex_unlock(&competitionMutex);
				if (inCompetition >= 10) 
				{
					pthread_mutex_lock(&isLeaderMutex);
				if (isLeader)
				{
					println("Kończę zawody");
					packet_t *pkt = malloc(sizeof(packet_t));
					for (int i = 0; i < size; i++) 
					{
						if (i != rank) sendPacket( pkt, i, END_COMPETITION );
					}
					free(pkt);
				}
				pthread_mutex_unlock(&isLeaderMutex);
				changeState( InRun );
				}
				break;
			default: 
				println("Nieznany stan")
				break;
		}
		sleep(SEC_IN_STATE);
    }
}
