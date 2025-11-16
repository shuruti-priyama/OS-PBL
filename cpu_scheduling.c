#include <stdio.h>
#include <stdlib.h>
#include "cpu_scheduling.h"

void fcfs_simulation() {
    int n;
    printf("Enter number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0) { 
        printf("Invalid number!\n"); 
        while(getchar() != '\n'); 
        return; 
    }

    int *burst = malloc(n * sizeof(int));
    int *wait = malloc(n * sizeof(int));
    int *turn = malloc(n * sizeof(int));
    if (!burst || !wait || !turn) { perror("malloc failed"); return; }

    printf("Enter burst times for %d processes:\n", n);
    for (int i = 0; i < n; i++) scanf("%d", &burst[i]);

    wait[0] = 0;
    for (int i = 1; i < n; i++) wait[i] = wait[i-1] + burst[i-1];
    for (int i = 0; i < n; i++) turn[i] = wait[i] + burst[i];

    printf("\nProcess\tBurst\tWait\tTurnaround\n");
    for (int i = 0; i < n; i++)
        printf("P%d\t%d\t%d\t%d\n", i+1, burst[i], wait[i], turn[i]);

    printf("\nGantt Chart:\n");
    for (int i = 0; i < n; i++) {
        printf("|");
        for (int j = 0; j < burst[i]; j++) printf("■");
    }
    printf("|\n");

    int current = 0;
    for (int i = 0; i < n; i++) {
        printf("%d\t", current);
        current += burst[i];
    }
    printf("%d\n", current);

    free(burst); free(wait); free(turn);
}
