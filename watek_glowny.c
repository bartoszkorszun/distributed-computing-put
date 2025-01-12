#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;

    while (state != InFinish) 
	{
		switch (state) 
		{
			case InRun: 
				perc = random()%100;
				if ( perc < 15 ) 
				{
					ackCount = 0;
					nackCount = 0;
					isInitiator = 1;
					println("Chcę się napić")
					packet_t *pkt = malloc(sizeof(packet_t));
					changeState( InWant ); 
					pkt->isInitiator = 1;
					for (int i = 0; i <= size-1; i++)
						if (i != rank) sendPacket( pkt, i, REQUEST );
					free(pkt);
					addMember(&myGroup, rank, lamportClock);
				} 
				break;
			case InWant:
				println("Czekam na utworzenie grupy")
				if (nackCount == size-1) 
				{
					packet_t *pkt = malloc(sizeof(packet_t));
					pkt->isInitiator = 0;
					isInitiator = 0;
				}
				break;
			case InGroup:
				println("Jestem w grupie")
				println("Członkowie grupy:");
				for (int i = 0; i < myGroup.groupSize; i++) 
				{
					printf(" - %d LC %d\n", myGroup.members[i], myGroup.timestamps[i]);
				}
				if (isInitiator)
				{
					for (int i = 0; i < initiatorsCount; i++) 
					{
						sendGroup( &myGroup, initiators[i], SGRP );
					}
				}
					
				// int leader = rank;
				// for (int i = 0; i < groupCount; i++)
				// {
				// 	for (int j = 0; j < groups[i].size; j++) 
				// 	{
                //     	if (groups[i].members[j] < leader) 
				// 		{
				// 			leader = groups[i].members[j];
				// 		}
                // 	}
				// }
				// if (rank == leader)
				// {
				// 	println("Jestem liderem grupy")
				// 	println("Członkowie grupy:");
				// 	for (int i = 0; i < groupCount; i++) 
				// 	{
				// 		for (int j = 0; j < groups[i].size; j++) 
				// 		{
				// 			println(" - %d", groups[i].members[j]);
				// 		}
				// 	}
				// 	pthread_mutex_lock(&arbiterMutex);
				// 	if (availableArbiters > 0)
				// 	{
				// 		availableArbiters--;
				// 		pthread_mutex_unlock(&arbiterMutex);
				// 		println("Rozpoczynam zawody")
				// 		sleep(10);
				// 		println("Napojeni to do akademika")
				// 		pthread_mutex_lock(&arbiterMutex);
				// 		availableArbiters++;
				// 		pthread_mutex_unlock(&arbiterMutex);
				// 		changeState( InRun );
				// 	}
				// 	else
				// 	{
				// 		pthread_mutex_unlock(&arbiterMutex);
				// 		println("Brak arbitrów, rozwiązuję grupę")
				// 		changeState( InRun );
				// 	}
				// }
				// else
				// {
				// 	sleep(10);
				// 	changeState( InRun );
				// }
				break;
			default: 
				break;
		}
		sleep(SEC_IN_STATE);
    }
}
