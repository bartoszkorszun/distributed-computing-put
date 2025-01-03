#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "lamport.h"
#include "arbitration.h"
#include "utils.h"

#define NUM_ARBITERS 5
#define TAG_INVITATION 1
#define TAG_RESPONSE 2
#define TAG_REQUEST_ARBITER 3
#define TAG_RELEASE_ARBITER 4

void process_function(int rank, int num_processes);

int main(int argc, char **argv) {
    int rank, num_processes;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    init_lamport_clock();
    init_arbitration(NUM_ARBITERS);

    process_function(rank, num_processes);

    cleanup_arbitration();
    MPI_Finalize();
    return 0;
}

void process_function(int rank, int num_processes) {
    while (1) {
        // Zgłaszanie chęci uczestnictwa w zawodach
        if (wants_to_participate()) {
            increment_clock();
            printf("Process %d broadcasting invitation (clock: %d)\n", rank, get_clock());

            // Wysłanie zaproszenia do wszystkich
            for (int i = 0; i < num_processes; i++) {
                if (i != rank) {
                    MPI_Send(&rank, 1, MPI_INT, i, TAG_INVITATION, MPI_COMM_WORLD);
                }
            }

            // Odbieranie odpowiedzi
            int ack_count = 0;
            for (int i = 0; i < num_processes - 1; i++) {
                int response;
                MPI_Recv(&response, 1, MPI_INT, MPI_ANY_SOURCE, TAG_RESPONSE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (response == 1) { // ACK
                    ack_count++;
                }
            }

            // Sprawdzenie, czy można rozpocząć zawody
            if (ack_count >= 2 && try_to_get_arbitrator(rank)) {
                participate_in_competition(rank);
                release_arbitrator(rank);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD); // Synchronizacja cykli
        sleep(rand() % 5 + 1); // Losowy czas oczekiwania
    }
}
