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
					packet_t *pkt = malloc(sizeof(packet_t));
					for (int i = 0; i <= size-1; i++)
						if (i != rank) sendPacket( pkt, i, REQUEST );
					free(pkt);
					group_t *myGroup = malloc(sizeof(group_t));
					addMember(myGroup, rank);
					changeState( InWant ); 
				} 
				break;
			case InWant:
				println("Czekam na utworzenie grupy")
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
