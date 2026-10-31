#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 20
#define MAX_NAME_LEN 16

// process states
typedef enum {
    STATE_NEW,
    STATE_READY,
    STATE_RUNNING,
    STATE_WAITING,
    STATE_TERMINATED
} ProcessState;

// Process Control Block
typedef struct {
    int pid;
    char name[MAX_NAME_LEN];
    int burst_time;         // total CPU burst time
    int remaining_time;     // remaining burst time (for preemptive algos)
    int arrival_time;
    int priority;           // lower number = higher priority
    ProcessState state;
    int waiting_time;
    int turnaround_time;
    int completion_time;
    int is_active;          // 1 if slot is used, 0 if free
} PCB;

// global process table
extern PCB process_table[MAX_PROCESSES];
extern int process_count;

// function declarations
void init_process_table(void);
int create_process(const char *name, int burst, int priority, int arrival);
int kill_process(int pid);
void list_processes(void);
const char* state_to_string(ProcessState s);
void reset_scheduling_fields(void);  // reset wait/turnaround times before scheduling

#endif
