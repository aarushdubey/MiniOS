# MiniOS Web UI Dashboard

An interactive browser-based dashboard for testing and visualizing all MiniOS operating system modules without manually typing terminal commands.

---

## 🚀 How to Run the UI

### Step 1: Start the UI Server
Open a terminal in the project directory and run:

```bash
python3 ui/app.py
```

*(Zero external dependencies required — runs using Python's standard library).*

### Step 2: Open in Browser
Visit the following URL in Google Chrome, Safari, Firefox, or Edge:

👉 **[http://127.0.0.1:5050](http://127.0.0.1:5050)**

---

## 🎯 Features in the UI

1. **Process Management**:
   - Add custom processes with Burst Time, Priority, and Arrival Time.
   - One-click button to load 5 standard textbook processes (`P1` to `P5`).
   - View process table and terminate processes by PID.

2. **CPU Scheduling**:
   - One-click execution for **FCFS**, **SJF**, **Priority**, and **MLFQ** with Gantt charts.
   - Configurable quantum for **Round Robin**.
   - **Compare All Schedulers** button with side-by-side performance table.

3. **Memory Management (1024 MB Arena)**:
   - Dynamic memory allocation with **First Fit (ff)**, **Best Fit (bf)**, and **Worst Fit (wf)**.
   - Free allocated blocks and automatically merge adjacent free memory.
   - Live visual memory map inspection.

4. **Process Synchronization & Deadlocks**:
   - Step-by-step **Producer-Consumer** bounded buffer simulation with semaphores.
   - Interactive **Banker's Algorithm** demonstrating resource safety algorithm and safe sequence.

5. **Page Replacement**:
   - Compare **FIFO**, **LRU**, and **Optimal** page replacement algorithms with custom frames and reference strings.

6. **Disk Scheduling**:
   - Simulate **FCFS**, **SSTF**, **SCAN (Elevator)**, and **C-SCAN** with custom head positions and request queues.

7. **Interactive Live Console**:
   - Real-time ANSI color output screen.
   - Built-in command line bar with command history (Up/Down arrow keys).
   - "Copy Output" and "Clear Screen" buttons.
   - "Run Full Demo" button to run the entire OS test suite in 5 seconds.
