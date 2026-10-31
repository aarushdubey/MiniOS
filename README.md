# MiniOS - Operating System Simulator

A terminal-based operating system simulator written in C that demonstrates core OS functionalities including process management, CPU scheduling, memory allocation, synchronization, page replacement and disk scheduling.

Built as a course project for **CSE302: Operating Systems**.

## Features

- **Process Management** - Create, list, and manage processes with PCB and state transitions
- **CPU Scheduling** - FCFS, SJF, Round Robin, Priority, and Multilevel Feedback Queue
- **Memory Management** - First Fit, Best Fit, Worst Fit allocation with memory map visualization
- **Synchronization** - Producer-Consumer problem and Banker's Algorithm for deadlock avoidance
- **Page Replacement** - FIFO, LRU, and Optimal algorithms with step-by-step simulation
- **Disk Scheduling** - FCFS, SSTF, SCAN, and C-SCAN algorithms

## How to Compile

### Linux / macOS
```bash
make
```

### Windows (MinGW)
```bash
gcc src/*.c -Iinclude -o minios.exe
```

## How to Run

```bash
./minios
```

Then use the interactive shell:
```
minios> help
minios> create P1 5 2
minios> list
minios> schedule rr
minios> exit
```

## Project Structure

```
os_project/
├── Makefile
├── README.md
├── include/
│   ├── process.h       # PCB struct and process management
│   ├── scheduler.h     # CPU scheduling algorithms
│   ├── memory.h        # Memory allocation
│   ├── sync.h          # Synchronization primitives
│   ├── pager.h         # Page replacement algorithms
│   └── disk.h          # Disk scheduling algorithms
├── src/
│   ├── main.c          # Shell and main loop
│   ├── process.c       # Process management implementation
│   ├── scheduler.c     # Scheduling algorithm implementations
│   ├── memory.c        # Memory manager implementation
│   ├── sync.c          # Sync primitives implementation
│   ├── pager.c         # Page replacement implementation
│   └── disk.c          # Disk scheduling implementation
└── docs/
    └── report.md
```

## Team

- Aarush Dubey
