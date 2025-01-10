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
					println("Chcę się napić")
					changeState( InWant ); 
				} 
				break;
			case InWant:
				println("Czekam na utworzenie grupy")
				packet_t *pkt = malloc(sizeof(packet_t));
				for (int i = 0; i <= size-1; i++)
					if (i != rank) sendPacket( pkt, i, REQUEST );
				free(pkt);
				// ackCount++;
				// if ( ackCount > 2) 
				// {
				// 	int inGroup = 0;
				// 	for (int i = 0; i < groupCount; i++)
				// 	{
				// 		for (int j = 0; j < groups[i].size; j++)
				// 		{
				// 			if (groups[i].members[j] == rank)
				// 			{
				// 				inGroup = 1;
				// 				break;
				// 			}
				// 		}
				// 		if (inGroup) break;
				// 	}
				// 	if (!inGroup) 
				// 	{
				// 		pthread_mutex_lock(&stateMut);
				// 		if (groupCount < MAX_GROUPS)
				// 		{
				// 			groups[groupCount].members[groups[groupCount].size++] = rank;
				// 			if (groups[groupCount].size >= 2) 
				// 			{
				// 				changeState(InGroup);
				// 			}
				// 		}
				// 		pthread_mutex_unlock(&stateMut);
				// 	}
				// }
				break;
			case InGroup:
				println("Jestem w grupie")
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
