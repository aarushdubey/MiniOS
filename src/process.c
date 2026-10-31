/*
 * process.c - Process Management Module
 * 
 * Handles creating processes (PCB), killing them,
 * listing the process table, and managing states.
 */

#include <stdio.h>
#include <string.h>
#include "process.h"

// color macros (same as main.c)
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

// global process table
PCB process_table[MAX_PROCESSES];
int process_count = 0;

void init_process_table(void) {
    memset(process_table, 0, sizeof(process_table));
    process_count = 0;
}

const char* state_to_string(ProcessState s) {
    switch (s) {
        case STATE_NEW:        return "NEW";
        case STATE_READY:      return "READY";
        case STATE_RUNNING:    return "RUNNING";
        case STATE_WAITING:    return "WAITING";
        case STATE_TERMINATED: return "TERMINATED";
        default:               return "UNKNOWN";
    }
}

int create_process(const char *name, int burst, int priority, int arrival) {
    if (process_count >= MAX_PROCESSES) {
        printf(RED "Error: Process table full (max %d)\n" RESET, MAX_PROCESSES);
        return -1;
    }
    
    if (burst <= 0) {
        printf(RED "Error: Burst time must be positive\n" RESET);
        return -1;
    }
    
    // check for duplicate name
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].is_active && strcmp(process_table[i].name, name) == 0) {
            printf(RED "Error: Process '%s' already exists\n" RESET, name);
            return -1;
        }
    }
    
    // find a free slot
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!process_table[i].is_active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        printf(RED "Error: No free slots\n" RESET);
        return -1;
    }
    
    // fill in the PCB
    process_table[slot].pid = slot;
    strncpy(process_table[slot].name, name, MAX_NAME_LEN - 1);
    process_table[slot].name[MAX_NAME_LEN - 1] = '\0';
    process_table[slot].burst_time = burst;
    process_table[slot].remaining_time = burst;
    process_table[slot].arrival_time = arrival;
    process_table[slot].priority = priority;
    process_table[slot].state = STATE_READY;
    process_table[slot].waiting_time = 0;
    process_table[slot].turnaround_time = 0;
    process_table[slot].completion_time = 0;
    process_table[slot].is_active = 1;
    
    process_count++;
    return slot;
}

int kill_process(int pid) {
    if (pid < 0 || pid >= MAX_PROCESSES) {
        printf(RED "Error: Invalid PID %d\n" RESET, pid);
        return -1;
    }
    if (!process_table[pid].is_active) {
        printf(RED "Error: Process %d does not exist\n" RESET, pid);
        return -1;
    }
    
    process_table[pid].state = STATE_TERMINATED;
    process_table[pid].is_active = 0;
    process_count--;
    return 0;
}

void list_processes(void) {
    if (process_count == 0) {
        printf(YELLOW "No processes in the system.\n" RESET);
        return;
    }
    
    printf(CYAN "\n%-5s %-12s %-8s %-10s %-10s %-8s %-10s\n" RESET,
           "PID", "Name", "Burst", "Remaining", "Arrival", "Priority", "State");
    printf("───────────────────────────────────────────────────────────────\n");
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].is_active) {
            printf("%-5d %-12s %-8d %-10d %-10d %-8d %-10s\n",
                   process_table[i].pid,
                   process_table[i].name,
                   process_table[i].burst_time,
                   process_table[i].remaining_time,
                   process_table[i].arrival_time,
                   process_table[i].priority,
                   state_to_string(process_table[i].state));
        }
    }
    printf("\nTotal processes: %d\n\n", process_count);
}

// reset wait/turnaround/completion times before running a scheduling algo
void reset_scheduling_fields(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].is_active) {
            process_table[i].remaining_time = process_table[i].burst_time;
            process_table[i].waiting_time = 0;
            process_table[i].turnaround_time = 0;
            process_table[i].completion_time = 0;
            process_table[i].state = STATE_READY;
        }
    }
}
