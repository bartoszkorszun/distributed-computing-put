#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;

    while (stan != InFinish) {
	switch (stan) {
	    case InRun: 
			perc = random()%100;
			if ( perc < 10 ) {
				println("Chcę się napić")
				packet_t *pkt = malloc(sizeof(packet_t));
				pkt->data = perc;
				ackCount = 0;
				for (int i=0;i<=size-1;i++)
					if (i!=rank)
						sendPacket( pkt, i, REQUEST);
				changeState( InWant ); 
				free(pkt);
			} // a skoro już jesteśmy przy komendach vi, najedź kursorem na } i wciśnij %  (niestety głupieje przy komentarzach :( )
			break;
	    case InWant:
			println("Czekam na utworzenie grupy")
			// tutaj zapewne jakiś semafor albo zmienna warunkowa
			// bo aktywne czekanie jest BUE
			ackCount++;
			if ( ackCount > 2) 
				changeState( InGroup );
			break;
	    case InGroup:
			// tutaj zapewne jakiś muteks albo zmienna warunkowa
			println("Jestem w grupie")
				sleep(5);
			println("Napojony kończę zawody i puszczam pawia lol")
			packet_t *pkt = malloc(sizeof(packet_t));
			pkt->data = perc;
			for (int i=0;i<=size-1;i++)
			if (i!=rank)
				sendPacket( pkt, (rank+1)%size, RELEASE);
			changeState( InRun );
			free(pkt);
			break;
	    default: 
			break;
        }

        sleep(SEC_IN_STATE);
    }
}
