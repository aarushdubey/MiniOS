# CSE302: Operating Systems Course Project Report

## Project Title: MiniOS — Modular Operating System Simulator

---

## 1. Abstract
MiniOS is a modular operating system simulator developed in C. It models and visualizes the core subsystems of a modern operating system: Process Lifecycle Management, CPU Scheduling, Dynamic Memory Allocation, Process Synchronization, Deadlock Avoidance, Virtual Memory Page Replacement, and Secondary Storage Disk Scheduling. MiniOS provides both an interactive command-line interface (CLI) and an interactive web dashboard for real-time demonstration and analysis.

---

## 2. System Architecture

MiniOS is structured into six independent, decoupled modules coordinated by a central shell:

```
MiniOS Core
 ├── Process Manager   (PCB, 5-state transitions)
 ├── CPU Scheduler     (FCFS, SJF, RR, Priority, MLFQ)
 ├── Memory Manager    (First Fit, Best Fit, Worst Fit, Coalescing)
 ├── Synchronization   (Producer-Consumer with Semaphores, Banker's Safety)
 ├── Virtual Memory    (FIFO, LRU, Optimal Page Replacement)
 └── Disk Scheduler    (FCFS, SSTF, SCAN, C-SCAN)
```

---

## 3. Module Details & Implementation

### 3.1 Process Management & PCB (`src/process.c`, `include/process.h`)
- Implements a Process Control Block (`PCB`) data structure containing:
  - Process ID (`pid`)
  - State (`NEW`, `READY`, `RUNNING`, `WAITING`, `TERMINATED`)
  - Timing metrics: Arrival Time, Burst Time, Remaining Time, Completion Time, Turnaround Time, and Waiting Time.
  - Priority level (integer scale).
- Supports dynamic creation, listing, state transitions, and process termination.

### 3.2 CPU Scheduling (`src/scheduler.c`, `include/scheduler.h`)
Implements five fundamental CPU scheduling algorithms:
1. **First-Come, First-Served (FCFS)**: Non-preemptive scheduling based on job arrival order.
2. **Shortest Job First (SJF)**: Non-preemptive greedy selection of shortest CPU bursts.
3. **Round Robin (RR)**: Preemptive time-sliced circular scheduling with configurable quantum.
4. **Priority Scheduling**: Non-preemptive selection prioritizing higher-priority processes.
5. **Multilevel Feedback Queue (MLFQ)**: 3-level queue hierarchy ($Q_0: \text{RR } q=1 \to Q_1: \text{RR } q=2 \to Q_2: \text{FCFS}$) with dynamic priority demotion.
6. **Side-by-side Comparative Analysis**: Renders ASCII Gantt charts and tabulates Average Waiting Time and Average Turnaround Time.

### 3.3 Dynamic Memory Management (`src/memory.c`, `include/memory.h`)
- Simulates a contiguous 1024 MB physical memory arena using a doubly-linked list of Memory Allocation Blocks (`MemBlock`).
- Allocation Policies:
  - **First Fit**: Allocates the first free hole of sufficient size.
  - **Best Fit**: Allocates the smallest free hole that accommodates the request (minimizes wasted block size).
  - **Worst Fit**: Allocates the largest available free hole.
- **Block Splitting & Coalescing**: On release, adjacent unallocated holes are merged forward and backward to prevent external fragmentation.

### 3.4 Synchronization & Deadlock Avoidance (`src/sync.c`, `include/sync.h`)
- **Producer-Consumer Problem**: Simulates multi-step bounded-buffer access using counting semaphores (`empty`, `full`) and a binary semaphore (`mutex`).
- **Banker's Algorithm**: Evaluates system safety for multiple resource types ($m$) across processes ($n$). Calculates the Need Matrix ($\text{Max} - \text{Allocation}$) and determines whether a safe sequence exists to avoid deadlock.

### 3.5 Virtual Memory Page Replacement (`src/pager.c`, `include/pager.h`)
- Simulates demand paging across a configurable number of physical frames.
- Implements:
  - **FIFO (First-In, First-Out)**: Replaces the oldest loaded page.
  - **LRU (Least Recently Used)**: Replaces the page untouched for the longest duration.
  - **Optimal (Belady's Min)**: Theoretical baseline looking ahead into future references to replace the page not needed for the longest interval.
- Outputs step-by-step frame residency, hit/fault status, total page faults, and hit ratio.

### 3.6 Disk Scheduling (`src/disk.c`, `include/disk.h`)
- Models head movement over 200 cylinders ($0-199$).
- Implements **FCFS**, **SSTF** (Shortest Seek Time First), **SCAN** (Elevator), and **C-SCAN** (Circular SCAN).
- Computes track traversal sequences and total seek distance in cylinders.

---

## 4. User Interfaces

1. **Terminal CLI Shell (`./minios`)**: Clean, interactive prompt with ANSI color coding and Gantt charts.
2. **Web UI Dashboard (`make ui` or `./run_ui.sh`)**: Browser dashboard (HTML5/CSS3/JavaScript) served via a lightweight, standard Python bridge (`ui/app.py`). Supports 1-click execution, module controls, and a 24-step automated demonstration.

---

## 5. Verification & Results

All algorithms were verified using standard textbook benchmarks (Silberschatz, Galvin & Gagne / Stallings):
- **CPU Scheduling Benchmark**: Process set $P_1(0,7), P_2(2,4), P_3(4,1), P_4(5,4), P_5(6,3)$ yielded SJF as the most optimal non-preemptive algorithm with an average waiting time of $4.80$ time units.
- **Page Replacement Benchmark**: Reference string `7,0,1,2,0,3,0,4,2,3,0,3` with 3 frames produced 10 faults (FIFO), 9 faults (LRU), and 7 faults (Optimal).
- **Disk Scheduling Benchmark**: Head at 53 with requests `98, 183, 37, 122, 14, 124, 65, 67` demonstrated SSTF achieving 236 cylinders seek time compared to 640 cylinders under FCFS.

---

## 6. Conclusion
MiniOS provides a clear, practical demonstration of core operating system abstractions. The implementation strictly adheres to standard C, avoiding external dependencies while delivering both terminal and graphical testing capabilities.

