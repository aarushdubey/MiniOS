/*
 * scheduler.c - CPU Scheduling Algorithms
 * 
 * Implements FCFS, SJF, Round Robin, Priority,
 * and Multilevel Feedback Queue scheduling.
 * Each algorithm works on the global process_table.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "process.h"
#include "scheduler.h"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define CYAN    "\033[1;36m"
#define MAGENTA "\033[1;35m"
#define RESET   "\033[0m"

// helper: collect active process indices into an array
// returns count
static int get_active_indices(int *indices) {
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].is_active) {
            indices[count++] = i;
        }
    }
    return count;
}

// print scheduling results table
static void print_results(int *indices, int n, const char *algo_name) {
    float total_wt = 0, total_tat = 0;
    
    printf(CYAN "\n--- %s Results ---\n" RESET, algo_name);
    printf("%-8s %-8s %-10s %-8s %-12s %-12s %-12s\n",
           "Name", "Arrival", "Burst", "Priority", "Completion", "Turnaround", "Waiting");
    printf("────────────────────────────────────────────────────────────────────────\n");
    
    for (int i = 0; i < n; i++) {
        int idx = indices[i];
        printf("%-8s %-8d %-10d %-8d %-12d %-12d %-12d\n",
               process_table[idx].name,
               process_table[idx].arrival_time,
               process_table[idx].burst_time,
               process_table[idx].priority,
               process_table[idx].completion_time,
               process_table[idx].turnaround_time,
               process_table[idx].waiting_time);
        total_wt += process_table[idx].waiting_time;
        total_tat += process_table[idx].turnaround_time;
    }
    
    printf("────────────────────────────────────────────────────────────────────────\n");
    printf(YELLOW "Average Turnaround Time: %.2f\n" RESET, total_tat / n);
    printf(YELLOW "Average Waiting Time   : %.2f\n\n" RESET, total_wt / n);
}

// print a gantt chart from arrays of process indices and time boundaries
void print_gantt_chart(int *order, int *start_times, int *end_times, int count) {
    printf(CYAN "\nGantt Chart:\n" RESET);
    
    // top border
    printf(" ");
    for (int i = 0; i < count; i++) {
        int width = (end_times[i] - start_times[i]) * 3;
        if (width < 4) width = 4;
        for (int j = 0; j < width; j++) printf("─");
        printf(" ");
    }
    printf("\n|");
    
    // process names
    for (int i = 0; i < count; i++) {
        int width = (end_times[i] - start_times[i]) * 3;
        if (width < 4) width = 4;
        int idx = order[i];
        char label[16];
        if (idx == -1)
            snprintf(label, sizeof(label), "idle");
        else
            snprintf(label, sizeof(label), "%s", process_table[idx].name);
        
        int pad = width - (int)strlen(label);
        int left_pad = pad / 2;
        int right_pad = pad - left_pad;
        for (int j = 0; j < left_pad; j++) printf(" ");
        printf("%s", label);
        for (int j = 0; j < right_pad; j++) printf(" ");
        printf("|");
    }
    printf("\n ");
    
    // bottom border
    for (int i = 0; i < count; i++) {
        int width = (end_times[i] - start_times[i]) * 3;
        if (width < 4) width = 4;
        for (int j = 0; j < width; j++) printf("─");
        printf(" ");
    }
    printf("\n");
    
    // time labels
    printf("%-3d", start_times[0]);
    for (int i = 0; i < count; i++) {
        int width = (end_times[i] - start_times[i]) * 3;
        if (width < 4) width = 4;
        // print end time
        char time_str[8];
        snprintf(time_str, sizeof(time_str), "%d", end_times[i]);
        // pad before the time label
        int pad = width + 1 - (int)strlen(time_str);
        for (int j = 0; j < pad; j++) printf(" ");
        printf("%s", time_str);
    }
    printf("\n\n");
}

/* ================== FCFS ================== */
void schedule_fcfs(void) {
    int indices[MAX_PROCESSES];
    int n = get_active_indices(indices);
    if (n == 0) return;
    
    reset_scheduling_fields();
    
    printf(MAGENTA "\n========== FCFS Scheduling ==========\n" RESET);
    
    // sort by arrival time (simple bubble sort)
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (process_table[indices[j]].arrival_time > process_table[indices[j + 1]].arrival_time) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    // simulate
    int gantt_order[100], gantt_start[100], gantt_end[100];
    int gantt_count = 0;
    int current_time = 0;
    
    for (int i = 0; i < n; i++) {
        int idx = indices[i];
        
        // if process hasn't arrived yet, CPU is idle
        if (current_time < process_table[idx].arrival_time) {
            gantt_order[gantt_count] = -1;
            gantt_start[gantt_count] = current_time;
            gantt_end[gantt_count] = process_table[idx].arrival_time;
            gantt_count++;
            current_time = process_table[idx].arrival_time;
        }
        
        gantt_order[gantt_count] = idx;
        gantt_start[gantt_count] = current_time;
        gantt_end[gantt_count] = current_time + process_table[idx].burst_time;
        gantt_count++;
        
        current_time += process_table[idx].burst_time;
        process_table[idx].completion_time = current_time;
        process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time;
        process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time;
        process_table[idx].state = STATE_TERMINATED;
    }
    
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count);
    print_results(indices, n, "FCFS");
}

/* ================== SJF (Non-preemptive) ================== */
void schedule_sjf(void) {
    int indices[MAX_PROCESSES];
    int n = get_active_indices(indices);
    if (n == 0) return;
    
    reset_scheduling_fields();
    
    printf(MAGENTA "\n========== SJF Scheduling (Non-Preemptive) ==========\n" RESET);
    
    int completed[MAX_PROCESSES] = {0};
    int done = 0;
    int current_time = 0;
    
    int gantt_order[100], gantt_start[100], gantt_end[100];
    int gantt_count = 0;
    
    while (done < n) {
        // find shortest job that has arrived and not completed
        int shortest = -1;
        int min_burst = INT_MAX;
        
        for (int i = 0; i < n; i++) {
            int idx = indices[i];
            if (!completed[i] && process_table[idx].arrival_time <= current_time) {
                if (process_table[idx].burst_time < min_burst) {
                    min_burst = process_table[idx].burst_time;
                    shortest = i;
                }
            }
        }
        
        if (shortest == -1) {
            // no process available, advance time
            current_time++;
            continue;
        }
        
        int idx = indices[shortest];
        gantt_order[gantt_count] = idx;
        gantt_start[gantt_count] = current_time;
        gantt_end[gantt_count] = current_time + process_table[idx].burst_time;
        gantt_count++;
        
        current_time += process_table[idx].burst_time;
        process_table[idx].completion_time = current_time;
        process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time;
        process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time;
        process_table[idx].state = STATE_TERMINATED;
        completed[shortest] = 1;
        done++;
    }
    
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count);
    print_results(indices, n, "SJF (Non-Preemptive)");
}

/* ================== Round Robin ================== */
void schedule_rr(int quantum) {
    int indices[MAX_PROCESSES];
    int n = get_active_indices(indices);
    if (n == 0) return;
    
    reset_scheduling_fields();
    
    printf(MAGENTA "\n========== Round Robin (quantum=%d) ==========\n" RESET, quantum);
    
    // sort by arrival time first
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (process_table[indices[j]].arrival_time > process_table[indices[j + 1]].arrival_time) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    // ready queue (circular)
    int queue[100], front = 0, rear = 0;
    int in_queue[MAX_PROCESSES] = {0};
    int remaining[MAX_PROCESSES];
    
    for (int i = 0; i < n; i++) {
        remaining[i] = process_table[indices[i]].burst_time;
    }
    
    int gantt_order[200], gantt_start[200], gantt_end[200];
    int gantt_count = 0;
    int current_time = 0;
    int done = 0;
    
    // add first process(es) that arrive at time 0
    for (int i = 0; i < n; i++) {
        if (process_table[indices[i]].arrival_time <= 0) {
            queue[rear++] = i;
            in_queue[i] = 1;
        }
    }
    
    while (done < n) {
        if (front == rear) {
            // queue empty, advance time to next arrival
            current_time++;
            for (int i = 0; i < n; i++) {
                if (!in_queue[i] && remaining[i] > 0 &&
                    process_table[indices[i]].arrival_time <= current_time) {
                    queue[rear++] = i;
                    in_queue[i] = 1;
                }
            }
            continue;
        }
        
        int qi = queue[front++];
        int idx = indices[qi];
        
        int exec_time = (remaining[qi] < quantum) ? remaining[qi] : quantum;
        
        gantt_order[gantt_count] = idx;
        gantt_start[gantt_count] = current_time;
        gantt_end[gantt_count] = current_time + exec_time;
        gantt_count++;
        
        current_time += exec_time;
        remaining[qi] -= exec_time;
        
        // check for new arrivals during this quantum
        for (int i = 0; i < n; i++) {
            if (!in_queue[i] && remaining[i] > 0 &&
                process_table[indices[i]].arrival_time <= current_time) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }
        
        if (remaining[qi] == 0) {
            // process finished
            process_table[idx].completion_time = current_time;
            process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time;
            process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time;
            process_table[idx].state = STATE_TERMINATED;
            done++;
        } else {
            // put back in queue
            queue[rear++] = qi;
        }
    }
    
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count);
    print_results(indices, n, "Round Robin");
}

/* ================== Priority (Non-preemptive) ================== */
void schedule_priority(void) {
    int indices[MAX_PROCESSES];
    int n = get_active_indices(indices);
    if (n == 0) return;
    
    reset_scheduling_fields();
    
    printf(MAGENTA "\n========== Priority Scheduling (Non-Preemptive) ==========\n" RESET);
    
    int completed[MAX_PROCESSES] = {0};
    int done = 0;
    int current_time = 0;
    
    int gantt_order[100], gantt_start[100], gantt_end[100];
    int gantt_count = 0;
    
    while (done < n) {
        // find highest priority (lowest number) process that has arrived
        int best = -1;
        int best_prio = INT_MAX;
        
        for (int i = 0; i < n; i++) {
            int idx = indices[i];
            if (!completed[i] && process_table[idx].arrival_time <= current_time) {
                if (process_table[idx].priority < best_prio) {
                    best_prio = process_table[idx].priority;
                    best = i;
                }
            }
        }
        
        if (best == -1) {
            current_time++;
            continue;
        }
        
        int idx = indices[best];
        gantt_order[gantt_count] = idx;
        gantt_start[gantt_count] = current_time;
        gantt_end[gantt_count] = current_time + process_table[idx].burst_time;
        gantt_count++;
        
        current_time += process_table[idx].burst_time;
        process_table[idx].completion_time = current_time;
        process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time;
        process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time;
        process_table[idx].state = STATE_TERMINATED;
        completed[best] = 1;
        done++;
    }
    
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count);
    print_results(indices, n, "Priority (Non-Preemptive)");
}

/* ================== MLFQ (3-level Feedback Queue) ================== */
void schedule_mlfq(void) {
    int indices[MAX_PROCESSES];
    int n = get_active_indices(indices);
    if (n == 0) return;
    
    reset_scheduling_fields();
    
    printf(MAGENTA "\n========== Multilevel Feedback Queue (3 levels) ==========\n" RESET);
    printf("Queue 0: RR q=1 | Queue 1: RR q=2 | Queue 2: FCFS\n\n");
    
    // sort by arrival time
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (process_table[indices[j]].arrival_time > process_table[indices[j + 1]].arrival_time) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    // 3 queues
    int q0[50], q0_front = 0, q0_rear = 0;  // highest priority, q=1
    int q1[50], q1_front = 0, q1_rear = 0;  // medium priority, q=2
    int q2[50], q2_front = 0, q2_rear = 0;  // lowest priority, FCFS
    
    int remaining[MAX_PROCESSES];
    int level[MAX_PROCESSES];  // which queue each process is in
    int arrived[MAX_PROCESSES] = {0};
    
    for (int i = 0; i < n; i++) {
        remaining[i] = process_table[indices[i]].burst_time;
        level[i] = 0;
    }
    
    int gantt_order[200], gantt_start[200], gantt_end[200];
    int gantt_count = 0;
    int current_time = 0;
    int done = 0;
    int quantum[] = {1, 2, 999};  // q2 is basically FCFS
    
    while (done < n) {
        // check for new arrivals
        for (int i = 0; i < n; i++) {
            if (!arrived[i] && process_table[indices[i]].arrival_time <= current_time) {
                q0[q0_rear++] = i;
                arrived[i] = 1;
            }
        }
        
        // pick from highest priority non-empty queue
        int qi = -1;
        int q_level = -1;
        
        if (q0_front < q0_rear) {
            qi = q0[q0_front++];
            q_level = 0;
        } else if (q1_front < q1_rear) {
            qi = q1[q1_front++];
            q_level = 1;
        } else if (q2_front < q2_rear) {
            qi = q2[q2_front++];
            q_level = 2;
        }
        
        if (qi == -1) {
            current_time++;
            continue;
        }
        
        int idx = indices[qi];
        int exec_time = (remaining[qi] < quantum[q_level]) ? remaining[qi] : quantum[q_level];
        
        gantt_order[gantt_count] = idx;
        gantt_start[gantt_count] = current_time;
        gantt_end[gantt_count] = current_time + exec_time;
        gantt_count++;
        
        current_time += exec_time;
        remaining[qi] -= exec_time;
        
        // check for arrivals during execution
        for (int i = 0; i < n; i++) {
            if (!arrived[i] && process_table[indices[i]].arrival_time <= current_time) {
                q0[q0_rear++] = i;
                arrived[i] = 1;
            }
        }
        
        if (remaining[qi] == 0) {
            process_table[idx].completion_time = current_time;
            process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time;
            process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time;
            process_table[idx].state = STATE_TERMINATED;
            done++;
        } else {
            // demote to next level
            if (q_level == 0) {
                level[qi] = 1;
                q1[q1_rear++] = qi;
            } else if (q_level == 1) {
                level[qi] = 2;
                q2[q2_rear++] = qi;
            } else {
                // already at lowest level, stay in q2
                q2[q2_rear++] = qi;
            }
        }
    }
    
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count);
    print_results(indices, n, "MLFQ");
}

/* ================== Compare All ================== */
void schedule_compare(void) {
    printf(MAGENTA "\n╔══════════════════════════════════════════╗\n");
    printf("║     Comparing All Scheduling Algorithms   ║\n");
    printf("╚══════════════════════════════════════════╝\n" RESET);
    
    // save original state
    PCB backup[MAX_PROCESSES];
    memcpy(backup, process_table, sizeof(process_table));
    
    // run each algorithm, collect avg WT and TAT
    float avg_wt[5], avg_tat[5];
    const char *names[] = {"FCFS", "SJF", "RR(q=2)", "Priority", "MLFQ"};
    int indices[MAX_PROCESSES];
    int n;
    
    // FCFS
    memcpy(process_table, backup, sizeof(process_table));
    schedule_fcfs();
    n = get_active_indices(indices);
    // recalculate from backup since processes are now terminated
    // actually let's just grab from the results that are stored
    {
        // restore to get indices
        memcpy(process_table, backup, sizeof(process_table));
        n = get_active_indices(indices);
        memcpy(process_table, backup, sizeof(process_table));
        schedule_fcfs();
        float tw = 0, tt = 0;
        for (int i = 0; i < n; i++) {
            tw += process_table[indices[i]].waiting_time;
            tt += process_table[indices[i]].turnaround_time;
        }
        avg_wt[0] = tw / n; avg_tat[0] = tt / n;
    }
    
    // SJF
    memcpy(process_table, backup, sizeof(process_table));
    schedule_sjf();
    {
        float tw = 0, tt = 0;
        for (int i = 0; i < n; i++) {
            tw += process_table[indices[i]].waiting_time;
            tt += process_table[indices[i]].turnaround_time;
        }
        avg_wt[1] = tw / n; avg_tat[1] = tt / n;
    }
    
    // RR
    memcpy(process_table, backup, sizeof(process_table));
    schedule_rr(2);
    {
        float tw = 0, tt = 0;
        for (int i = 0; i < n; i++) {
            tw += process_table[indices[i]].waiting_time;
            tt += process_table[indices[i]].turnaround_time;
        }
        avg_wt[2] = tw / n; avg_tat[2] = tt / n;
    }
    
    // Priority
    memcpy(process_table, backup, sizeof(process_table));
    schedule_priority();
    {
        float tw = 0, tt = 0;
        for (int i = 0; i < n; i++) {
            tw += process_table[indices[i]].waiting_time;
            tt += process_table[indices[i]].turnaround_time;
        }
        avg_wt[3] = tw / n; avg_tat[3] = tt / n;
    }
    
    // MLFQ
    memcpy(process_table, backup, sizeof(process_table));
    schedule_mlfq();
    {
        float tw = 0, tt = 0;
        for (int i = 0; i < n; i++) {
            tw += process_table[indices[i]].waiting_time;
            tt += process_table[indices[i]].turnaround_time;
        }
        avg_wt[4] = tw / n; avg_tat[4] = tt / n;
    }
    
    // print comparison table
    printf(CYAN "\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║              Algorithm Comparison Summary              ║\n");
    printf("╠═══════════════╦═══════════════════╦═══════════════════╣\n");
    printf("║  Algorithm    ║  Avg Wait Time    ║  Avg Turnaround   ║\n");
    printf("╠═══════════════╬═══════════════════╬═══════════════════╣\n" RESET);
    
    // find best (lowest) avg WT
    float best_wt = avg_wt[0];
    int best_wt_idx = 0;
    for (int i = 1; i < 5; i++) {
        if (avg_wt[i] < best_wt) { best_wt = avg_wt[i]; best_wt_idx = i; }
    }
    
    for (int i = 0; i < 5; i++) {
        if (i == best_wt_idx)
            printf(GREEN);
        printf("║  %-12s ║      %8.2f     ║      %8.2f     ║\n",
               names[i], avg_wt[i], avg_tat[i]);
        if (i == best_wt_idx)
            printf(RESET);
    }
    printf(CYAN "╚═══════════════╩═══════════════════╩═══════════════════╝\n" RESET);
    printf(GREEN "\nBest Average Waiting Time: %s (%.2f)\n\n" RESET, names[best_wt_idx], best_wt);
    
    // restore original process table
    memcpy(process_table, backup, sizeof(process_table));
}
