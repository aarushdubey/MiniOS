#ifndef DISK_H
#define DISK_H

#define MAX_REQUESTS 30
#define DISK_SIZE 200  // cylinders 0 to 199

// disk scheduling algorithms
void disk_fcfs(int *requests, int n, int head);
void disk_sstf(int *requests, int n, int head);
void disk_scan(int *requests, int n, int head);
void disk_cscan(int *requests, int n, int head);

// compare all disk algorithms
void disk_compare(int *requests, int n, int head);

#endif
