#include "arbitration.h"

int num_arbiters;
int *arbiters;
pthread_mutex_t arbiter_mutex;

void init_arbitration(int num) {
    num_arbiters = num;
    arbiters = malloc(num * sizeof(int));
    for (int i = 0; i < num; i++) {
        arbiters[i] = 0; // Arbiter wolny
    }
    pthread_mutex_init(&arbiter_mutex, NULL);
}

int try_to_get_arbitrator(int rank) {
    pthread_mutex_lock(&arbiter_mutex);
    for (int i = 0; i < num_arbiters; i++) {
        if (arbiters[i] == 0) {
            arbiters[i] = rank;
            pthread_mutex_unlock(&arbiter_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&arbiter_mutex);
    return 0;
}

void release_arbitrator(int rank) {
    pthread_mutex_lock(&arbiter_mutex);
    for (int i = 0; i < num_arbiters; i++) {
        if (arbiters[i] == rank) {
            arbiters[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&arbiter_mutex);
}

void cleanup_arbitration() {
    free(arbiters);
    pthread_mutex_destroy(&arbiter_mutex);
}
