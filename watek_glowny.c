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
					sgrpCount = 0;
					rgrpCount = 0;
					isInitiator = 1;
					println("Chcę się napić")
					packet_t *pkt = malloc(sizeof(packet_t));
					changeState( InWant ); 
					for (int i = 0; i <= size-1; i++)
						if (i != rank) sendPacket( pkt, i, REQUEST );
					free(pkt);
					addMember(rank, lamportClock);
				} 
				break;
			case InWant:
				println("Czekam na utworzenie grupy")
				if (nackCount == size-1) 
				{
					isInitiator = 0;
				}
				break;
			case InGroup:
				if (isInitiator)
				{
					packet_t *gpkt = malloc(sizeof(packet_t));
					pthread_mutex_lock(&groupPacketMutex);
					for (int i = 0; i < initiatorsCount; i++) 
					{
						sendGroup( gpkt, initiators[i], SGRP );
					}
					isInitiator = 0;
					pthread_mutex_unlock(&groupPacketMutex);
					free(gpkt);
				}	
				if (isGroupFormed) 
				{
					println("Grupa utworzona");
					println("Członkowie grupy:");
					for (int i = 0; i < myGroup.groupSize; i++) 
					{
						printf(" - %d LC %d\n", myGroup.members[i], myGroup.timestamps[i]);
					}
					chooseLeader();
					if (isLeader) 
					{
						println("Jestem liderem");
					}
				}
				break;
			default: 
				break;
		}
		sleep(SEC_IN_STATE);
    }
}
