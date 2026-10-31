/*
 * disk.c - Disk Scheduling Algorithms
 *
 * Implements FCFS, SSTF, SCAN, and C-SCAN disk scheduling.
 * Shows the order of disk head movement and total seek time.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define CYAN    "\033[1;36m"
#define MAGENTA "\033[1;35m"
#define RESET   "\033[0m"

// absolute value helper
static int abs_val(int x) {
    return (x < 0) ? -x : x;
}

// print the order of disk head movement
static void print_seek_order(int *order, int count, int total_seek) {
    printf(CYAN "  Seek Order: " RESET);
    for (int i = 0; i < count; i++) {
        printf("%d", order[i]);
        if (i < count - 1) printf(" -> ");
    }
    printf("\n");
    printf(YELLOW "  Total Seek Time: %d cylinders\n\n" RESET, total_seek);
}

/* ============ FCFS Disk Scheduling ============ */
void disk_fcfs(int *requests, int n, int head) {
    printf(MAGENTA "\n========== FCFS Disk Scheduling ==========\n" RESET);
    printf("  Initial Head Position: %d\n\n", head);
    
    int order[MAX_REQUESTS + 1];
    int total_seek = 0;
    int current = head;
    
    order[0] = head;
    for (int i = 0; i < n; i++) {
        total_seek += abs_val(requests[i] - current);
        current = requests[i];
        order[i + 1] = current;
    }
    
    print_seek_order(order, n + 1, total_seek);
}

/* ============ SSTF Disk Scheduling ============ */
void disk_sstf(int *requests, int n, int head) {
    printf(MAGENTA "\n========== SSTF Disk Scheduling ==========\n" RESET);
    printf("  Initial Head Position: %d\n\n", head);
    
    int visited[MAX_REQUESTS] = {0};
    int order[MAX_REQUESTS + 1];
    int total_seek = 0;
    int current = head;
    
    order[0] = head;
    
    for (int i = 0; i < n; i++) {
        // find nearest unvisited request
        int min_dist = DISK_SIZE + 1;
        int nearest = -1;
        
        for (int j = 0; j < n; j++) {
            if (!visited[j]) {
                int dist = abs_val(requests[j] - current);
                if (dist < min_dist) {
                    min_dist = dist;
                    nearest = j;
                }
            }
        }
        
        if (nearest == -1) break;
        
        visited[nearest] = 1;
        total_seek += min_dist;
        current = requests[nearest];
        order[i + 1] = current;
    }
    
    print_seek_order(order, n + 1, total_seek);
}

/* ============ SCAN (Elevator) Disk Scheduling ============ */
void disk_scan(int *requests, int n, int head) {
    printf(MAGENTA "\n========== SCAN (Elevator) Disk Scheduling ==========\n" RESET);
    printf("  Initial Head Position: %d  (moving towards higher end)\n\n", head);
    
    // sort requests
    int sorted[MAX_REQUESTS];
    memcpy(sorted, requests, n * sizeof(int));
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (sorted[j] > sorted[j + 1]) {
                int temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    int order[MAX_REQUESTS + 10];
    int count = 0;
    int total_seek = 0;
    
    order[count++] = head;
    
    // find where head fits in sorted array
    int start = 0;
    while (start < n && sorted[start] < head) start++;
    
    // go right first (towards higher cylinders)
    for (int i = start; i < n; i++) {
        order[count++] = sorted[i];
    }
    
    // go to the end of disk
    if (sorted[n - 1] != DISK_SIZE - 1 && start < n) {
        order[count++] = DISK_SIZE - 1;
    }
    
    // now go left
    for (int i = start - 1; i >= 0; i--) {
        order[count++] = sorted[i];
    }
    
    // calculate total seek
    int current = head;
    for (int i = 1; i < count; i++) {
        total_seek += abs_val(order[i] - current);
        current = order[i];
    }
    
    print_seek_order(order, count, total_seek);
}

/* ============ C-SCAN Disk Scheduling ============ */
void disk_cscan(int *requests, int n, int head) {
    printf(MAGENTA "\n========== C-SCAN Disk Scheduling ==========\n" RESET);
    printf("  Initial Head Position: %d  (moving towards higher end)\n\n", head);
    
    // sort requests
    int sorted[MAX_REQUESTS];
    memcpy(sorted, requests, n * sizeof(int));
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (sorted[j] > sorted[j + 1]) {
                int temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    int order[MAX_REQUESTS + 10];
    int count = 0;
    int total_seek = 0;
    
    order[count++] = head;
    
    int start = 0;
    while (start < n && sorted[start] < head) start++;
    
    // go right
    for (int i = start; i < n; i++) {
        order[count++] = sorted[i];
    }
    
    // go to end, then jump to beginning
    order[count++] = DISK_SIZE - 1;
    order[count++] = 0;
    
    // continue from beginning going right
    for (int i = 0; i < start; i++) {
        order[count++] = sorted[i];
    }
    
    // calculate total seek
    int current = head;
    for (int i = 1; i < count; i++) {
        total_seek += abs_val(order[i] - current);
        current = order[i];
    }
    
    print_seek_order(order, count, total_seek);
}

/* ============ Compare All Disk Algorithms ============ */
void disk_compare(int *requests, int n, int head) {
    printf(MAGENTA "\n╔══════════════════════════════════════════╗\n");
    printf("║    Comparing Disk Scheduling Algorithms   ║\n");
    printf("╚══════════════════════════════════════════╝\n" RESET);
    
    disk_fcfs(requests, n, head);
    disk_sstf(requests, n, head);
    disk_scan(requests, n, head);
    disk_cscan(requests, n, head);
}
