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
            // JEŻELI CHCĘ BRAĆ UDZIAŁ W ZAWODACH I NIE JESTEM W GRUPIE 
            // TO WYSYŁAM ACK DO PROCESU KTÓRY WYSŁAŁ MI ZAPYTANIE
            // I ZMIENIAM STAN NA InGroup
            // W PRZECIWNYM PRZYPADKU WYSYŁAM NACK
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
            // JEŻELI JESTEM W GRUPIE TO ZAPISUJĘ DO GRUPY NOWYCH CZŁONKÓW
            // JEŻELI NIE JESTEM W GRUPIE TO ZAPISUJĘ DO GRUPY NOWYCH CZŁONKÓW
            // I ZMIENIAM STAN NA InGroup
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
            // JEŻELI JESTEM W STANIE InWant TO LICZĘ NACKI ŻEBY PÓŹNIEJ OKREŚLIĆ
            // CZY JESTEM INICJATOREM
            case NACK:
                if (state == InWant) 
                {
                    nackCount++;
                }
                break;
            // JEŻELI JESTEM INICJATOREM TO MOGĘ OTRZYMAĆ GRUPĘ OD INNEGO INICJATORA
            // SPRAWDZAM CZŁONKÓW GRUPY I ŁĄCZĘ JE W JEDNĄ GRUPĘ
            // WYSYŁAM DO WSZYSTKICH CZŁONKÓW GRUPY WIADOMOŚĆ O POWSTANIU GRUPY
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
            // JEŻELI INICJATORZY SIĘ DOGADALI WYSYŁAJĄ WIADOMOŚĆ RGRP DO WSZYSTKICH CZŁONKÓW GRUPY
            // CZŁONKOWIE AKTUALIZUJĄ SWOJE GRUPY
            // JEŻELI ILOŚĆ TAKICH WIADOMOŚĆI JEST RÓWNA ILOŚCI INICJAOTRÓW TO GRUPA JEST GOTOWA
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
                // TODO
                if (rgrpCount == myGroup.groupSize - 1) 
                {
                    isGroupFormed = 1;
                }
                break;
            // ZAPYTANIE O WOLNYCH ARBITRÓW
            // JEŻELI JESTEM LIDEREM I BIORĘ UDZIAŁ W ZAWODACH TO WYSYŁAM NACK
            // W PRZECIWNYM PRZYPADKU WYSYŁAM ACK
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
            // JEŻELI OTRZYMUJĘ TAKI KOMUNIKAT, TO OZNACZA, ŻE JESTEM LIDEREM
            // SPRAWDZAM, CZY LICZBA ACK POZWALA NA ROZPOCZĘCIE ZAWODÓW
            // JEŻELI TAK TO SPRAWDZAM, CZY INNY LIDER NIE ZGŁOSIŁ SIĘ DO ZAWODÓW SZYBCIEJ
            // JEŻELI TAK TO SPRAWDZAM KTO MA NAJNIŻSZY TIMESTAMP I JEŻELI TO JA TO ZACZYNAM ZAWODY
            // ZMIENIAM STAN NA InCompetition
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
            // JEŻELI OTRZYMUJĘ TAKI KOMUNIKAT, TO OZNACZA, ŻE JESTEM LIDEREM
            // LICZĘ NACK
            case NACK_ARBITERS:
                pthread_mutex_lock(&nackArbiterMutex);
                nackArbitersCount++;
                pthread_mutex_unlock(&nackArbiterMutex);
                break;
            // KAŻDY PROCES OTRZYMUJE TAKI KOMUNIKAT OD LIDERA, KTÓRY ZACZYNA ZAWODY
            // JEŻELI JESTEM W GRUPIE, SPRAWDZAM, CZY TO MÓJ LIDER ZACZYNA ZAWODY
            // JEŻELI TAK TO ZMIENIAM STAN NA InCompetition
            // JEŻELI RÓWNIEŻ JESTEM LIDEREM I OCZEKUJĘ NA ARBITRA TO USUWAM TEGO LIDERA Z LISTY OCZEKUJĄCYCH
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
                if (isAskingForArbiter && isLeader)
                {
                    removeOtherLeader( status.MPI_SOURCE );
                }
                break;
            // LIDER PO ZAKOŃCZENIU ZAWODÓW WYSYŁA TAKĄ WIADOMOŚĆ
            // JEŻELI JESTEM LIDEREM I ZAWODY SIĘ SKOŃCZYŁY TO SPRAWDZAM, CZY MOGĘ ZACZĄĆ ZAWODY
            // JEŻELI TAK TO ZMIENIAM STAN NA InCompetition
            // I WYSYŁAM WIADOMOŚĆ DO WSZYSTKICH PROCESÓW O ROZPOCZĘCIU ZAWODÓW
            case END_COMPETITION:
                if (isLeader) 
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
            default:
                break;
            }
    }
}
