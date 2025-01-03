#include "utils.h"
#include <stdio.h>
#include <unistd.h>

int wants_to_participate() {
    return rand() % 2; // 50% szans na chęć uczestnictwa
}

void participate_in_competition(int rank) {
    printf("Process %d is participating in the competition!\n", rank);
    sleep(2); // Symulacja czasu trwania zawodów
}
