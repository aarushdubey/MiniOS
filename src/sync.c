/*
 * sync.c - Synchronization Module
 * 
 * Simulates Producer-Consumer problem with semaphores
 * and implements Banker's Algorithm for deadlock avoidance.
 * 
 * Note: This is a step-by-step simulation (not actual threads).
 * We simulate the semaphore wait/signal operations and show
 * what happens at each step.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sync.h"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define CYAN    "\033[1;36m"
#define MAGENTA "\033[1;35m"
#define RESET   "\033[0m"

/* ============ Producer-Consumer Simulation ============ */

void run_producer_consumer(void) {
    printf(MAGENTA "\n========== Producer-Consumer Problem ==========\n" RESET);
    printf("Buffer size: %d | Simulating 10 steps\n\n", BUFFER_SIZE);
    
    int buffer[BUFFER_SIZE];
    /* count tracks items in buffer (used conceptually) */
    int in_idx = 0;     // producer index
    int out_idx = 0;    // consumer index
    int item = 1;       // item counter
    
    // simulated semaphores
    int empty = BUFFER_SIZE;  // counts empty slots
    int full = 0;             // counts full slots
    int mutex = 1;            // binary semaphore for mutual exclusion
    
    // init buffer
    for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = 0;
    
    printf(CYAN "%-6s %-12s %-8s %-8s %-8s  Buffer State\n" RESET,
           "Step", "Action", "empty", "full", "mutex");
    printf("──────────────────────────────────────────────────────────\n");
    
    // simulate a sequence of produce/consume operations
    // pattern: P P P C P C P C C C (mix of operations)
    char *actions[] = {"produce", "produce", "produce", "consume",
                       "produce", "consume", "produce", "consume",
                       "consume", "consume"};
    int num_steps = 10;
    
    for (int step = 0; step < num_steps; step++) {
        char buf_str[64] = "[";
        
        if (strcmp(actions[step], "produce") == 0) {
            // producer wants to produce
            if (empty <= 0) {
                printf("%-6d " RED "BLOCKED" RESET "     (buffer full, producer waits)\n", step + 1);
                // build buffer string for display
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    char tmp[8];
                    snprintf(tmp, sizeof(tmp), "%d", buffer[i]);
                    strcat(buf_str, tmp);
                    if (i < BUFFER_SIZE - 1) strcat(buf_str, ",");
                }
                strcat(buf_str, "]");
                continue;
            }
            
            // wait(empty) -> decrement
            empty--;
            // wait(mutex) -> acquire
            mutex = 0;
            
            // produce item
            buffer[in_idx] = item;
            in_idx = (in_idx + 1) % BUFFER_SIZE;
            // signal(mutex) -> release
            mutex = 1;
            // signal(full) -> increment
            full++;
            
            // build buffer string
            for (int i = 0; i < BUFFER_SIZE; i++) {
                char tmp[8];
                if (buffer[i] == 0) snprintf(tmp, sizeof(tmp), "_");
                else snprintf(tmp, sizeof(tmp), "%d", buffer[i]);
                strcat(buf_str, tmp);
                if (i < BUFFER_SIZE - 1) strcat(buf_str, ",");
            }
            strcat(buf_str, "]");
            
            printf("%-6d " GREEN "Produce %-4d" RESET " %-8d %-8d %-8d  %s\n",
                   step + 1, item, empty, full, mutex, buf_str);
            item++;
        }
        else {
            // consumer wants to consume
            if (full <= 0) {
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    char tmp[8];
                    if (buffer[i] == 0) snprintf(tmp, sizeof(tmp), "_");
                    else snprintf(tmp, sizeof(tmp), "%d", buffer[i]);
                    strcat(buf_str, tmp);
                    if (i < BUFFER_SIZE - 1) strcat(buf_str, ",");
                }
                strcat(buf_str, "]");
                printf("%-6d " RED "BLOCKED" RESET "      (buffer empty, consumer waits)  %s\n",
                       step + 1, buf_str);
                continue;
            }
            
            // wait(full) -> decrement
            full--;
            // wait(mutex)
            mutex = 0;
            
            // consume item
            int consumed = buffer[out_idx];
            buffer[out_idx] = 0;
            out_idx = (out_idx + 1) % BUFFER_SIZE;
            // signal(mutex)
            mutex = 1;
            // signal(empty)
            empty++;
            
            for (int i = 0; i < BUFFER_SIZE; i++) {
                char tmp[8];
                if (buffer[i] == 0) snprintf(tmp, sizeof(tmp), "_");
                else snprintf(tmp, sizeof(tmp), "%d", buffer[i]);
                strcat(buf_str, tmp);
                if (i < BUFFER_SIZE - 1) strcat(buf_str, ",");
            }
            strcat(buf_str, "]");
            
            printf("%-6d " YELLOW "Consume %-4d" RESET " %-8d %-8d %-8d  %s\n",
                   step + 1, consumed, empty, full, mutex, buf_str);
        }
    }
    printf("\n");
}

/* ============ Banker's Algorithm ============ */

void run_bankers(void) {
    printf(MAGENTA "\n========== Banker's Algorithm ==========\n" RESET);
    
    int n, m;  // n = processes, m = resource types
    
    printf("Enter number of processes (max %d): ", MAX_SYNC_PROC);
    if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_SYNC_PROC) {
        printf(RED "Invalid input\n" RESET);
        while (getchar() != '\n');
        return;
    }
    
    printf("Enter number of resource types (max %d): ", MAX_RESOURCES);
    if (scanf("%d", &m) != 1 || m <= 0 || m > MAX_RESOURCES) {
        printf(RED "Invalid input\n" RESET);
        while (getchar() != '\n');
        return;
    }
    
    int alloc[MAX_SYNC_PROC][MAX_RESOURCES];
    int max[MAX_SYNC_PROC][MAX_RESOURCES];
    int avail[MAX_RESOURCES];
    int need[MAX_SYNC_PROC][MAX_RESOURCES];
    
    printf("\nEnter Allocation Matrix (%d x %d):\n", n, m);
    for (int i = 0; i < n; i++) {
        printf("  P%d: ", i);
        for (int j = 0; j < m; j++) {
            scanf("%d", &alloc[i][j]);
        }
    }
    
    printf("Enter Max Matrix (%d x %d):\n", n, m);
    for (int i = 0; i < n; i++) {
        printf("  P%d: ", i);
        for (int j = 0; j < m; j++) {
            scanf("%d", &max[i][j]);
        }
    }
    
    printf("Enter Available Resources (%d values): ", m);
    for (int j = 0; j < m; j++) {
        scanf("%d", &avail[j]);
    }
    
    // clear the newline
    while (getchar() != '\n');
    
    // calculate Need matrix = Max - Allocation
    printf(CYAN "\n--- Need Matrix (Max - Allocation) ---\n" RESET);
    printf("     ");
    for (int j = 0; j < m; j++) printf("R%-3d", j);
    printf("\n");
    
    for (int i = 0; i < n; i++) {
        printf("P%d:  ", i);
        for (int j = 0; j < m; j++) {
            need[i][j] = max[i][j] - alloc[i][j];
            printf("%-4d", need[i][j]);
        }
        printf("\n");
    }
    
    // Safety Algorithm
    int work[MAX_RESOURCES];
    int finish[MAX_SYNC_PROC] = {0};
    int safe_seq[MAX_SYNC_PROC];
    int seq_count = 0;
    
    for (int j = 0; j < m; j++) work[j] = avail[j];
    
    printf(CYAN "\n--- Safety Algorithm Execution ---\n" RESET);
    
    int found;
    do {
        found = 0;
        for (int i = 0; i < n; i++) {
            if (finish[i]) continue;
            
            // check if need[i] <= work
            int can_run = 1;
            for (int j = 0; j < m; j++) {
                if (need[i][j] > work[j]) {
                    can_run = 0;
                    break;
                }
            }
            
            if (can_run) {
                printf("  P%d can run: Need(", i);
                for (int j = 0; j < m; j++) {
                    printf("%d", need[i][j]);
                    if (j < m - 1) printf(",");
                }
                printf(") <= Work(");
                for (int j = 0; j < m; j++) {
                    printf("%d", work[j]);
                    if (j < m - 1) printf(",");
                }
                printf(")\n");
                
                // simulate: work = work + alloc[i]
                for (int j = 0; j < m; j++) {
                    work[j] += alloc[i][j];
                }
                
                printf("         Work updated to (");
                for (int j = 0; j < m; j++) {
                    printf("%d", work[j]);
                    if (j < m - 1) printf(",");
                }
                printf(")\n");
                
                finish[i] = 1;
                safe_seq[seq_count++] = i;
                found = 1;
            }
        }
    } while (found);
    
    // check result
    if (seq_count == n) {
        printf(GREEN "\nSystem is in a SAFE state!\n" RESET);
        printf(GREEN "Safe Sequence: < ");
        for (int i = 0; i < n; i++) {
            printf("P%d", safe_seq[i]);
            if (i < n - 1) printf(" -> ");
        }
        printf(" >\n\n" RESET);
    } else {
        printf(RED "\nSystem is in an UNSAFE state! Deadlock possible.\n" RESET);
        printf(RED "Processes that cannot finish: ");
        for (int i = 0; i < n; i++) {
            if (!finish[i]) printf("P%d ", i);
        }
        printf("\n\n" RESET);
    }
}
