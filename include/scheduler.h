#ifndef SCHEDULER_H
#define SCHEDULER_H

// run different scheduling algorithms
void schedule_fcfs(void);
void schedule_sjf(void);
void schedule_rr(int quantum);
void schedule_priority(void);
void schedule_mlfq(void);

// compare all algorithms side by side
void schedule_compare(void);

// helper to print gantt chart
void print_gantt_chart(int *order, int *start_times, int *end_times, int count);

#endif
