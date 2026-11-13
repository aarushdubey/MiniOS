// MiniOS Simulator Dashboard Logic
// Supports both:
// 1. Local mode (communicating with compiled C binary via Python app.py on 127.0.0.1:5050)
// 2. Standalone Web / GitHub Pages mode (pure in-browser C-equivalent OS simulation engine)

const terminal = document.getElementById("terminalOutput");
const cliInput = document.getElementById("cliInput");
const statusDot = document.querySelector(".status-dot");
const statusText = document.getElementById("statusText");
const fullDemoBtn = document.getElementById("fullDemoBtn");
const terminalTitle = document.getElementById("terminalTitle");

// Detection of local server vs static GitHub Pages
let useLocalServer = window.location.hostname === "127.0.0.1" || window.location.hostname === "localhost";

// Command history
let cmdHistory = [];
let historyIndex = -1;

// Demo state
let isDemoRunning = false;

// ANSI escape sequence to HTML converter
function ansiToHtml(text) {
  if (!text) return "";
  
  let safe = text
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;");

  safe = safe
    .replace(/\033\[1;31m/g, '<span class="ansi-red">')
    .replace(/\033\[1;32m/g, '<span class="ansi-green">')
    .replace(/\033\[1;33m/g, '<span class="ansi-yellow">')
    .replace(/\033\[1;34m/g, '<span class="ansi-blue">')
    .replace(/\033\[1;35m/g, '<span class="ansi-magenta">')
    .replace(/\033\[1;36m/g, '<span class="ansi-cyan">')
    .replace(/\033\[1;37m/g, '<span class="ansi-white">')
    .replace(/\033\[0m/g, '</span>');

  return safe;
}

function appendToTerminal(rawText, isUserInput = false, isStepBanner = false) {
  const block = document.createElement("div");
  if (isStepBanner) {
    block.innerHTML = `<div class="step-banner">${rawText}</div>`;
  } else if (isUserInput) {
    block.innerHTML = `<span style="color:#06b6d4;font-weight:600;">minios&gt;</span> <span style="color:#f8fafc;">${rawText}</span>`;
  } else {
    block.innerHTML = ansiToHtml(rawText);
  }
  
  terminal.appendChild(block);
  terminal.scrollTop = terminal.scrollHeight;
}

function setStatus(text, isBusy = false) {
  statusText.innerText = text;
  if (isBusy) {
    statusDot.style.backgroundColor = "#eab308";
    statusDot.style.boxShadow = "0 0 8px #eab308";
  } else {
    statusDot.style.backgroundColor = "#22c55e";
    statusDot.style.boxShadow = "0 0 8px #22c55e";
  }
}

function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

function toggleCard(header) {
  const card = header.closest(".card");
  if (card) {
    card.classList.toggle("collapsed");
  }
}

// ============================================================================
// STANDALONE IN-BROWSER SIMULATION ENGINE (Identical to MiniOS C Implementation)
// ============================================================================

const StandaloneOS = {
  processes: [],
  memoryBlocks: [{ offset: 0, size: 1024, allocated: false, processName: "" }],

  reset() {
    this.processes = [];
    this.memoryBlocks = [{ offset: 0, size: 1024, allocated: false, processName: "" }];
  },

  createProcess(name, burst, priority, arrival) {
    burst = parseInt(burst);
    priority = parseInt(priority);
    arrival = parseInt(arrival) || 0;

    if (isNaN(burst) || burst <= 0) {
      return "\033[1;31mError: Burst time must be positive\033[0m\n";
    }
    if (this.processes.some((p) => p.name === name)) {
      return `\033[1;31mError: Process '${name}' already exists\033[0m\n`;
    }

    const pid = this.processes.length;
    this.processes.push({
      pid,
      name,
      burst,
      remaining: burst,
      priority,
      arrival,
      state: "READY",
      waitingTime: 0,
      turnaroundTime: 0,
      completionTime: 0,
    });
    return `\033[1;32mProcess '${name}' created with PID ${pid}\033[0m\n`;
  },

  listProcesses() {
    if (this.processes.length === 0) {
      return "\033[1;33mNo processes in the system.\033[0m\n";
    }
    let out = "\033[1;36m\nPID   Name         Burst    Remaining  Arrival    Priority State     \n\033[0m";
    out += "───────────────────────────────────────────────────────────────\n";
    for (const p of this.processes) {
      const pidStr = String(p.pid).padEnd(5);
      const nameStr = p.name.padEnd(12);
      const burstStr = String(p.burst).padEnd(8);
      const remStr = String(p.remaining).padEnd(10);
      const arrStr = String(p.arrival).padEnd(10);
      const prioStr = String(p.priority).padEnd(8);
      out += `${pidStr} ${nameStr} ${burstStr} ${remStr} ${arrStr} ${prioStr} ${p.state}\n`;
    }
    out += `\nTotal processes: ${this.processes.length}\n\n`;
    return out;
  },

  killProcess(pid) {
    pid = parseInt(pid);
    const idx = this.processes.findIndex((p) => p.pid === pid);
    if (idx === -1) {
      return `\033[1;31mError: Process ${pid} does not exist\033[0m\n`;
    }
    this.processes.splice(idx, 1);
    return `\033[1;32mProcess ${pid} terminated.\033[0m\n`;
  },

  clearProcesses() {
    this.processes = [];
    return "\033[1;32mAll processes cleared.\033[0m\n";
  },

  // CPU Schedulers
  buildGanttChart(order, startTimes, endTimes) {
    let out = "\033[1;36m\nGantt Chart:\n\033[0m ";
    for (let i = 0; i < order.length; i++) {
      let width = Math.max(4, (endTimes[i] - startTimes[i]) * 3);
      out += "─".repeat(width) + " ";
    }
    out += "\n|";
    for (let i = 0; i < order.length; i++) {
      let width = Math.max(4, (endTimes[i] - startTimes[i]) * 3);
      let label = order[i];
      let left = Math.floor((width - label.length) / 2);
      let right = width - label.length - left;
      out += " ".repeat(left) + label + " ".repeat(right) + "|";
    }
    out += "\n ";
    for (let i = 0; i < order.length; i++) {
      let width = Math.max(4, (endTimes[i] - startTimes[i]) * 3);
      out += "─".repeat(width) + " ";
    }
    out += "\n" + String(startTimes[0]).padEnd(3);
    for (let i = 0; i < order.length; i++) {
      let width = Math.max(4, (endTimes[i] - startTimes[i]) * 3);
      let timeStr = String(endTimes[i]);
      let pad = width + 1 - timeStr.length;
      out += " ".repeat(Math.max(1, pad)) + timeStr;
    }
    out += "\n\n";
    return out;
  },

  buildResultsTable(procs, algoName) {
    let totalWt = 0, totalTat = 0;
    let out = `\033[1;36m--- ${algoName} Results ---\n\033[0m`;
    out += "Name     Arrival  Burst      Priority Completion   Turnaround   Waiting     \n";
    out += "────────────────────────────────────────────────────────────────────────\n";
    for (const p of procs) {
      out += `${p.name.padEnd(8)} ${String(p.arrival).padEnd(8)} ${String(p.burst).padEnd(10)} ${String(p.priority).padEnd(8)} ${String(p.completionTime).padEnd(12)} ${String(p.turnaroundTime).padEnd(12)} ${String(p.waitingTime).padEnd(12)}\n`;
      totalWt += p.waitingTime;
      totalTat += p.turnaroundTime;
    }
    out += "────────────────────────────────────────────────────────────────────────\n";
    const avgTat = (totalTat / procs.length).toFixed(2);
    const avgWt = (totalWt / procs.length).toFixed(2);
    out += `\033[1;33mAverage Turnaround Time: ${avgTat}\nAverage Waiting Time   : ${avgWt}\n\n\033[0m`;
    return { out, avgWt: parseFloat(avgWt), avgTat: parseFloat(avgTat) };
  },

  scheduleFCFS() {
    if (this.processes.length === 0) return "\033[1;31mError: No processes created.\033[0m\n";
    const procs = JSON.parse(JSON.stringify(this.processes)).sort((a, b) => a.arrival - b.arrival);
    let curTime = 0, order = [], startTimes = [], endTimes = [];

    for (const p of procs) {
      if (curTime < p.arrival) {
        order.push("idle");
        startTimes.push(curTime);
        endTimes.push(p.arrival);
        curTime = p.arrival;
      }
      order.push(p.name);
      startTimes.push(curTime);
      endTimes.push(curTime + p.burst);
      curTime += p.burst;
      p.completionTime = curTime;
      p.turnaroundTime = p.completionTime - p.arrival;
      p.waitingTime = p.turnaroundTime - p.burst;
    }

    let res = "\033[1;35m\n========== FCFS Scheduling ==========\n\033[0m";
    res += this.buildGanttChart(order, startTimes, endTimes);
    const table = this.buildResultsTable(procs, "FCFS");
    return { text: res + table.out, avgWt: table.avgWt, avgTat: table.avgTat };
  },

  scheduleSJF() {
    if (this.processes.length === 0) return "\033[1;31mError: No processes created.\033[0m\n";
    const procs = JSON.parse(JSON.stringify(this.processes));
    let completed = new Array(procs.length).fill(false);
    let done = 0, curTime = 0, order = [], startTimes = [], endTimes = [];

    while (done < procs.length) {
      let shortest = -1, minBurst = Infinity;
      for (let i = 0; i < procs.length; i++) {
        if (!completed[i] && procs[i].arrival <= curTime && procs[i].burst < minBurst) {
          minBurst = procs[i].burst;
          shortest = i;
        }
      }
      if (shortest === -1) {
        curTime++;
        continue;
      }
      const p = procs[shortest];
      order.push(p.name);
      startTimes.push(curTime);
      endTimes.push(curTime + p.burst);
      curTime += p.burst;
      p.completionTime = curTime;
      p.turnaroundTime = p.completionTime - p.arrival;
      p.waitingTime = p.turnaroundTime - p.burst;
      completed[shortest] = true;
      done++;
    }

    let res = "\033[1;35m\n========== SJF Scheduling (Non-Preemptive) ==========\n\033[0m";
    res += this.buildGanttChart(order, startTimes, endTimes);
    const table = this.buildResultsTable(procs, "SJF (Non-Preemptive)");
    return { text: res + table.out, avgWt: table.avgWt, avgTat: table.avgTat };
  },

  scheduleRR(quantum = 2) {
    if (this.processes.length === 0) return "\033[1;31mError: No processes created.\033[0m\n";
    const procs = JSON.parse(JSON.stringify(this.processes)).sort((a, b) => a.arrival - b.arrival);
    let remaining = procs.map((p) => p.burst);
    let curTime = 0, done = 0, queue = [], inQueue = new Array(procs.length).fill(false);
    let order = [], startTimes = [], endTimes = [];

    for (let i = 0; i < procs.length; i++) {
      if (procs[i].arrival <= 0) {
        queue.push(i);
        inQueue[i] = true;
      }
    }

    while (done < procs.length) {
      if (queue.length === 0) {
        curTime++;
        for (let i = 0; i < procs.length; i++) {
          if (!inQueue[i] && remaining[i] > 0 && procs[i].arrival <= curTime) {
            queue.push(i);
            inQueue[i] = true;
          }
        }
        continue;
      }
      const idx = queue.shift();
      const p = procs[idx];
      const exec = Math.min(remaining[idx], quantum);

      order.push(p.name);
      startTimes.push(curTime);
      endTimes.push(curTime + exec);
      curTime += exec;
      remaining[idx] -= exec;

      for (let i = 0; i < procs.length; i++) {
        if (!inQueue[i] && remaining[i] > 0 && procs[i].arrival <= curTime) {
          queue.push(i);
          inQueue[i] = true;
        }
      }

      if (remaining[idx] === 0) {
        p.completionTime = curTime;
        p.turnaroundTime = p.completionTime - p.arrival;
        p.waitingTime = p.turnaroundTime - p.burst;
        done++;
      } else {
        queue.push(idx);
      }
    }

    let res = `\033[1;35m\n========== Round Robin (quantum=${quantum}) ==========\n\033[0m`;
    res += this.buildGanttChart(order, startTimes, endTimes);
    const table = this.buildResultsTable(procs, "Round Robin");
    return { text: res + table.out, avgWt: table.avgWt, avgTat: table.avgTat };
  },

  schedulePriority() {
    if (this.processes.length === 0) return "\033[1;31mError: No processes created.\033[0m\n";
    const procs = JSON.parse(JSON.stringify(this.processes));
    let completed = new Array(procs.length).fill(false);
    let done = 0, curTime = 0, order = [], startTimes = [], endTimes = [];

    while (done < procs.length) {
      let best = -1, bestPrio = Infinity;
      for (let i = 0; i < procs.length; i++) {
        if (!completed[i] && procs[i].arrival <= curTime && procs[i].priority < bestPrio) {
          bestPrio = procs[i].priority;
          best = i;
        }
      }
      if (best === -1) {
        curTime++;
        continue;
      }
      const p = procs[best];
      order.push(p.name);
      startTimes.push(curTime);
      endTimes.push(curTime + p.burst);
      curTime += p.burst;
      p.completionTime = curTime;
      p.turnaroundTime = p.completionTime - p.arrival;
      p.waitingTime = p.turnaroundTime - p.burst;
      completed[best] = true;
      done++;
    }

    let res = "\033[1;35m\n========== Priority Scheduling (Non-Preemptive) ==========\n\033[0m";
    res += this.buildGanttChart(order, startTimes, endTimes);
    const table = this.buildResultsTable(procs, "Priority (Non-Preemptive)");
    return { text: res + table.out, avgWt: table.avgWt, avgTat: table.avgTat };
  },

  scheduleMLFQ() {
    if (this.processes.length === 0) return "\033[1;31mError: No processes created.\033[0m\n";
    const procs = JSON.parse(JSON.stringify(this.processes)).sort((a, b) => a.arrival - b.arrival);
    let q0 = [], q1 = [], q2 = [];
    let remaining = procs.map((p) => p.burst);
    let arrived = new Array(procs.length).fill(false);
    let curTime = 0, done = 0, order = [], startTimes = [], endTimes = [];
    const quanta = [1, 2, 9999];

    while (done < procs.length) {
      for (let i = 0; i < procs.length; i++) {
        if (!arrived[i] && procs[i].arrival <= curTime) {
          q0.push(i);
          arrived[i] = true;
        }
      }

      let qi = -1, level = -1;
      if (q0.length > 0) { qi = q0.shift(); level = 0; }
      else if (q1.length > 0) { qi = q1.shift(); level = 1; }
      else if (q2.length > 0) { qi = q2.shift(); level = 2; }

      if (qi === -1) {
        curTime++;
        continue;
      }

      const p = procs[qi];
      const exec = Math.min(remaining[qi], quanta[level]);

      order.push(p.name);
      startTimes.push(curTime);
      endTimes.push(curTime + exec);
      curTime += exec;
      remaining[qi] -= exec;

      for (let i = 0; i < procs.length; i++) {
        if (!arrived[i] && procs[i].arrival <= curTime) {
          q0.push(i);
          arrived[i] = true;
        }
      }

      if (remaining[qi] === 0) {
        p.completionTime = curTime;
        p.turnaroundTime = p.completionTime - p.arrival;
        p.waitingTime = p.turnaroundTime - p.burst;
        done++;
      } else {
        if (level === 0) q1.push(qi);
        else q2.push(qi);
      }
    }

    let res = "\033[1;35m\n========== Multilevel Feedback Queue (3 levels) ==========\n\033[0m";
    res += "Queue 0: RR q=1 | Queue 1: RR q=2 | Queue 2: FCFS\n\n";
    res += this.buildGanttChart(order, startTimes, endTimes);
    const table = this.buildResultsTable(procs, "MLFQ");
    return { text: res + table.out, avgWt: table.avgWt, avgTat: table.avgTat };
  },

  scheduleCompare() {
    if (this.processes.length === 0) return "\033[1;31mError: No processes created. Use 'create' first.\033[0m\n";

    let out = "\033[1;35m\n╔══════════════════════════════════════════╗\n║     Comparing All Scheduling Algorithms   ║\n╚══════════════════════════════════════════╝\n\033[0m";
    const fcfs = this.scheduleFCFS();
    const sjf = this.scheduleSJF();
    const rr = this.scheduleRR(2);
    const prio = this.schedulePriority();
    const mlfq = this.scheduleMLFQ();

    out += fcfs.text + sjf.text + rr.text + prio.text + mlfq.text;

    const names = ["FCFS", "SJF", "RR(q=2)", "Priority", "MLFQ"];
    const wts = [fcfs.avgWt, sjf.avgWt, rr.avgWt, prio.avgWt, mlfq.avgWt];
    const tats = [fcfs.avgTat, sjf.avgTat, rr.avgTat, prio.avgTat, mlfq.avgTat];

    let bestIdx = 0;
    for (let i = 1; i < wts.length; i++) {
      if (wts[i] < wts[bestIdx]) bestIdx = i;
    }

    out += "\033[1;36m\n╔═══════════════════════════════════════════════════════╗\n";
    out += "║              Algorithm Comparison Summary              ║\n";
    out += "╠═══════════════╦═══════════════════╦═══════════════════╣\n";
    out += "║  Algorithm    ║  Avg Wait Time    ║  Avg Turnaround   ║\n";
    out += "╠═══════════════╬═══════════════════╬═══════════════════╣\n\033[0m";

    for (let i = 0; i < 5; i++) {
      let isBest = i === bestIdx;
      let line = `║  ${names[i].padEnd(12)} ║      ${wts[i].toFixed(2).padStart(8)}     ║      ${tats[i].toFixed(2).padStart(8)}     ║\n`;
      out += isBest ? `\033[1;32m${line}\033[0m` : line;
    }
    out += "\033[1;36m╚═══════════════╩═══════════════════╩═══════════════════╝\n\033[0m";
    out += `\033[1;32m\nBest Average Waiting Time: ${names[bestIdx]} (${wts[bestIdx].toFixed(2)})\n\n\033[0m`;
    return out;
  },

  // Memory Management (1024 MB arena)
  memAlloc(pname, size, policy) {
    size = parseInt(size);
    if (isNaN(size) || size <= 0 || size > 1024) {
      return "\033[1;31mError: Invalid memory size (must be 1-1024 MB)\033[0m\n";
    }
    if (this.memoryBlocks.some((b) => b.allocated && b.processName === pname)) {
      return `\033[1;31mError: '${pname}' already has memory allocated\033[0m\n`;
    }

    let chosenIdx = -1;
    if (policy === "ff") {
      chosenIdx = this.memoryBlocks.findIndex((b) => !b.allocated && b.size >= size);
    } else if (policy === "bf") {
      let minSize = Infinity;
      for (let i = 0; i < this.memoryBlocks.length; i++) {
        const b = this.memoryBlocks[i];
        if (!b.allocated && b.size >= size && b.size < minSize) {
          minSize = b.size;
          chosenIdx = i;
        }
      }
    } else if (policy === "wf") {
      let maxSize = -1;
      for (let i = 0; i < this.memoryBlocks.length; i++) {
        const b = this.memoryBlocks[i];
        if (!b.allocated && b.size >= size && b.size > maxSize) {
          maxSize = b.size;
          chosenIdx = i;
        }
      }
    }

    if (chosenIdx === -1) {
      return `\033[1;31mError: Not enough contiguous memory for ${size} MB\033[0m\n`;
    }

    const block = this.memoryBlocks[chosenIdx];
    const offset = block.offset;
    const policyName = policy === "ff" ? "First Fit" : policy === "bf" ? "Best Fit" : "Worst Fit";

    if (block.size === size) {
      block.allocated = true;
      block.processName = pname;
    } else {
      const remainingSize = block.size - size;
      block.size = size;
      block.allocated = true;
      block.processName = pname;
      this.memoryBlocks.splice(chosenIdx + 1, 0, {
        offset: offset + size,
        size: remainingSize,
        allocated: false,
        processName: "",
      });
    }

    return `\033[1;32m  [${policyName}] Allocated at offset ${offset}\nAllocated ${size} MB to '${pname}'\033[0m\n`;
  },

  memFree(pname) {
    const idx = this.memoryBlocks.findIndex((b) => b.allocated && b.processName === pname);
    if (idx === -1) {
      return `\033[1;31mError: No memory allocated to '${pname}'\033[0m\n`;
    }
    this.memoryBlocks[idx].allocated = false;
    this.memoryBlocks[idx].processName = "";

    // Coalesce adjacent free blocks
    for (let i = 0; i < this.memoryBlocks.length - 1; ) {
      if (!this.memoryBlocks[i].allocated && !this.memoryBlocks[i + 1].allocated) {
        this.memoryBlocks[i].size += this.memoryBlocks[i + 1].size;
        this.memoryBlocks.splice(i + 1, 1);
      } else {
        i++;
      }
    }
    return `\033[1;32mFreed memory for '${pname}'\033[0m\n`;
  },

  memStatus() {
    let out = "\033[1;36m\n=== Memory Map (Total: 1024 MB) ===\n\033[0m────────────────────────────────────────────────\n";
    let allocTotal = 0, freeTotal = 0;
    for (const b of this.memoryBlocks) {
      const start = String(b.offset).padStart(4);
      const end = String(b.offset + b.size - 1).padStart(4);
      const name = b.allocated ? b.processName.padEnd(10) : "---".padEnd(10);
      const sz = String(b.size).padStart(4);
      if (b.allocated) {
        out += `\033[1;32m  [${start} - ${end}]  ${name}  ${sz} MB  ALLOCATED\033[0m\n`;
        allocTotal += b.size;
      } else {
        out += `\033[1;33m  [${start} - ${end}]  ${name}  ${sz} MB  FREE\033[0m\n`;
        freeTotal += b.size;
      }
    }
    out += `────────────────────────────────────────────────\n  Allocated: ${allocTotal} MB | Free: ${freeTotal} MB\n\n`;
    return out;
  },

  producerConsumer() {
    let out = "\033[1;35m\n========== Producer-Consumer Problem ==========\n\033[0m";
    out += "Buffer size: 5 | Simulating 10 steps\n\n";
    out += "\033[1;36mStep   Action       empty    full     mutex     Buffer State\n\033[0m──────────────────────────────────────────────────────────\n";

    const buffer = ["_", "_", "_", "_", "_"];
    let empty = 5, full = 0, item = 1, inIdx = 0, outIdx = 0;
    const actions = ["P", "P", "P", "C", "P", "C", "P", "C", "C", "C"];

    for (let step = 0; step < actions.length; step++) {
      const act = actions[step];
      const stepStr = String(step + 1).padEnd(6);
      if (act === "P") {
        empty--;
        buffer[inIdx] = String(item);
        inIdx = (inIdx + 1) % 5;
        full++;
        out += `${stepStr} \033[1;32mProduce ${String(item).padEnd(4)}\033[0m ${String(empty).padEnd(8)} ${String(full).padEnd(8)} 1         [${buffer.join(",")}]\n`;
        item++;
      } else {
        full--;
        const consumed = buffer[outIdx];
        buffer[outIdx] = "_";
        outIdx = (outIdx + 1) % 5;
        empty++;
        out += `${stepStr} \033[1;33mConsume ${consumed.padEnd(4)}\033[0m ${String(empty).padEnd(8)} ${String(full).padEnd(8)} 1         [${buffer.join(",")}]\n`;
      }
    }
    out += "\n";
    return out;
  },

  bankers() {
    let out = "\033[1;35m\n========== Banker's Algorithm ==========\n\033[0m";
    out += "\033[1;36m\n--- Need Matrix (Max - Allocation) ---\n\033[0m";
    out += "     R0  R1  R2  \n";
    out += "P0:  7   4   3   \nP1:  1   2   2   \nP2:  6   0   0   \nP3:  0   1   1   \nP4:  4   3   1   \n";
    out += "\033[1;36m\n--- Safety Algorithm Execution ---\n\033[0m";
    out += "  P1 can run: Need(1,2,2) <= Work(3,3,2)\n         Work updated to (5,3,2)\n";
    out += "  P3 can run: Need(0,1,1) <= Work(5,3,2)\n         Work updated to (7,4,3)\n";
    out += "  P4 can run: Need(4,3,1) <= Work(7,4,3)\n         Work updated to (7,4,5)\n";
    out += "  P0 can run: Need(7,4,3) <= Work(7,4,5)\n         Work updated to (7,5,5)\n";
    out += "  P2 can run: Need(6,0,0) <= Work(7,5,5)\n         Work updated to (10,5,7)\n";
    out += "\033[1;32m\nSystem is in a SAFE state!\n\033[0m";
    out += "\033[1;32mSafe Sequence: < P1 -> P3 -> P4 -> P0 -> P2 >\n\n\033[0m";
    return out;
  },

  pageReplace(algo, frames, ref) {
    frames = parseInt(frames);
    const pages = ref.split(",").map((s) => parseInt(s.trim())).filter((n) => !isNaN(n));
    if (pages.length === 0) return "\033[1;31mError: Empty reference string\033[0m\n";

    if (algo === "compare") {
      let out = "\033[1;35m\n╔══════════════════════════════════════════╗\n║   Comparing Page Replacement Algorithms   ║\n╚══════════════════════════════════════════╝\n\033[0m";
      out += this.pageReplace("fifo", frames, ref);
      out += this.pageReplace("lru", frames, ref);
      out += this.pageReplace("optimal", frames, ref);
      out += "\033[1;36mNote: Optimal is theoretical (requires future knowledge)\n      and gives the minimum possible page faults.\n\n\033[0m";
      return out;
    }

    const title = algo.toUpperCase();
    let out = `\033[1;35m\n========== ${title} Page Replacement (${frames} frames) ==========\n\033[0m`;
    out += "\033[1;36mRef      Frames          Status    \n\033[0m─────────────────────────────────\n";

    let frameArr = new Array(frames).fill("-");
    let faults = 0;
    let fifoPtr = 0;
    let lastUsed = new Array(frames).fill(-1);

    for (let i = 0; i < pages.length; i++) {
      const p = pages[i];
      const inFrame = frameArr.indexOf(p);
      const refStr = `  ${String(p).padEnd(6)}`;

      if (inFrame !== -1) {
        lastUsed[inFrame] = i;
        out += `${refStr} [ ${frameArr.map((v) => String(v).padStart(2)).join(", ")} ]  \033[1;32mHIT\033[0m\n`;
      } else {
        faults++;
        if (algo === "fifo") {
          frameArr[fifoPtr] = p;
          fifoPtr = (fifoPtr + 1) % frames;
        } else if (algo === "lru") {
          let lruIdx = 0, minTime = lastUsed[0];
          for (let j = 1; j < frames; j++) {
            if (lastUsed[j] < minTime) { minTime = lastUsed[j]; lruIdx = j; }
          }
          frameArr[lruIdx] = p;
          lastUsed[lruIdx] = i;
        } else if (algo === "optimal") {
          const emptySlot = frameArr.indexOf("-");
          if (emptySlot !== -1) {
            frameArr[emptySlot] = p;
          } else {
            let farthest = -1, replaceIdx = 0;
            for (let j = 0; j < frames; j++) {
              let nextUse = pages.slice(i + 1).indexOf(frameArr[j]);
              if (nextUse === -1) nextUse = 9999;
              if (nextUse > farthest) { farthest = nextUse; replaceIdx = j; }
            }
            frameArr[replaceIdx] = p;
          }
        }
        out += `${refStr} [ ${frameArr.map((v) => String(v).padStart(2)).join(", ")} ]  \033[1;31mFAULT\033[0m\n`;
      }
    }

    out += "─────────────────────────────────\n";
    out += `\033[1;33mTotal Page Faults: ${faults} / ${pages.length}\n`;
    out += `Hit Ratio: ${(((pages.length - faults) / pages.length) * 100).toFixed(2)}%\n\n\033[0m`;
    return out;
  },

  diskSchedule(algo, head, reqStr) {
    head = parseInt(head);
    const reqs = reqStr.split(",").map((s) => parseInt(s.trim())).filter((n) => !isNaN(n));
    if (reqs.length === 0) return "\033[1;31mError: Empty request list\033[0m\n";

    if (algo === "compare") {
      let out = "\033[1;35m\n╔══════════════════════════════════════════╗\n║    Comparing Disk Scheduling Algorithms   ║\n╚══════════════════════════════════════════╝\n\033[0m";
      out += this.diskSchedule("fcfs", head, reqStr);
      out += this.diskSchedule("sstf", head, reqStr);
      out += this.diskSchedule("scan", head, reqStr);
      out += this.diskSchedule("cscan", head, reqStr);
      return out;
    }

    let order = [head], totalSeek = 0;

    if (algo === "fcfs") {
      let cur = head;
      for (const r of reqs) {
        totalSeek += Math.abs(r - cur);
        cur = r;
        order.push(cur);
      }
    } else if (algo === "sstf") {
      let visited = new Array(reqs.length).fill(false);
      let cur = head;
      for (let i = 0; i < reqs.length; i++) {
        let minDist = Infinity, nearest = -1;
        for (let j = 0; j < reqs.length; j++) {
          if (!visited[j]) {
            const dist = Math.abs(reqs[j] - cur);
            if (dist < minDist) { minDist = dist; nearest = j; }
          }
        }
        if (nearest === -1) break;
        visited[nearest] = true;
        totalSeek += minDist;
        cur = reqs[nearest];
        order.push(cur);
      }
    } else if (algo === "scan") {
      const sorted = [...reqs].sort((a, b) => a - b);
      let start = sorted.findIndex((r) => r >= head);
      if (start === -1) start = sorted.length;
      for (let i = start; i < sorted.length; i++) order.push(sorted[i]);
      if (sorted[sorted.length - 1] !== 199 && start < sorted.length) order.push(199);
      for (let i = start - 1; i >= 0; i--) order.push(sorted[i]);

      for (let i = 1; i < order.length; i++) totalSeek += Math.abs(order[i] - order[i - 1]);
    } else if (algo === "cscan") {
      const sorted = [...reqs].sort((a, b) => a - b);
      let start = sorted.findIndex((r) => r >= head);
      if (start === -1) start = sorted.length;
      for (let i = start; i < sorted.length; i++) order.push(sorted[i]);
      order.push(199);
      order.push(0);
      for (let i = 0; i < start; i++) order.push(sorted[i]);

      for (let i = 1; i < order.length; i++) totalSeek += Math.abs(order[i] - order[i - 1]);
    }

    const title = algo.toUpperCase();
    let out = `\033[1;35m\n========== ${title} Disk Scheduling ==========\n\033[0m`;
    out += `  Initial Head Position: ${head}\n\n`;
    out += `\033[1;36m  Seek Order: \033[0m${order.join(" -> ")}\n`;
    out += `\033[1;33m  Total Seek Time: ${totalSeek} cylinders\n\n\033[0m`;
    return out;
  },

  executeCommand(cmd) {
    const parts = cmd.trim().split(/\s+/);
    const action = parts[0];

    if (action === "create") {
      return this.createProcess(parts[1], parts[2], parts[3], parts[4]);
    } else if (action === "list") {
      return this.listProcesses();
    } else if (action === "kill") {
      return this.killProcess(parts[1]);
    } else if (action === "clear_all") {
      return this.clearProcesses();
    } else if (action === "schedule") {
      const algo = parts[1];
      if (algo === "fcfs") return this.scheduleFCFS().text;
      if (algo === "sjf") return this.scheduleSJF().text;
      if (algo === "rr") return this.scheduleRR(parseInt(parts[2]) || 2).text;
      if (algo === "priority") return this.schedulePriority().text;
      if (algo === "mlfq") return this.scheduleMLFQ().text;
      if (algo === "compare") return this.scheduleCompare();
      return `\033[1;31mUnknown algorithm: ${algo}\033[0m\n`;
    } else if (action === "mem_alloc") {
      return this.memAlloc(parts[1], parts[2], parts[3]);
    } else if (action === "mem_free") {
      return this.memFree(parts[1]);
    } else if (action === "mem_status") {
      return this.memStatus();
    } else if (action === "mem_reset") {
      this.memoryBlocks = [{ offset: 0, size: 1024, allocated: false, processName: "" }];
      return "\033[1;32mMemory reset.\033[0m\n";
    } else if (action === "producer_consumer") {
      return this.producerConsumer();
    } else if (action === "bankers") {
      return this.bankers();
    } else if (action === "page") {
      return this.pageReplace(parts[1], parts[2], parts[3]);
    } else if (action === "disk") {
      return this.diskSchedule(parts[1], parts[2], parts[3]);
    } else if (action === "help") {
      return "\033[1;33m\nCommands: create, list, kill, schedule (fcfs|sjf|rr|priority|mlfq|compare), mem_alloc, mem_free, mem_status, producer_consumer, bankers, page, disk, clear_all\033[0m\n";
    }
    return `\033[1;31mUnknown command: '${action}'\033[0m\n`;
  },
};

// ============================================================================
// COMMAND DISPATCHER (Local Server with Standalone In-Browser Fallback)
// ============================================================================

async function runCommand(cmd, timeoutSeconds = 6.0) {
  if (!cmd || !cmd.trim()) return;
  const cleanCmd = cmd.trim();

  appendToTerminal(cleanCmd, true);
  setStatus("Executing...", true);

  // If local server mode is active, try local API
  if (useLocalServer) {
    try {
      const res = await fetch("/api/cmd", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ command: cleanCmd, timeout: timeoutSeconds }),
      });
      if (res.ok) {
        const data = await res.json();
        if (data.output) {
          appendToTerminal(data.output);
          return data.output;
        }
      } else {
        useLocalServer = false; // switch to client-side fallback
      }
    } catch {
      useLocalServer = false; // switch to client-side fallback
    }
  }

  // Standalone in-browser execution
  const out = StandaloneOS.executeCommand(cleanCmd);
  appendToTerminal(out);

  if (!isDemoRunning) {
    setStatus("System Ready", false);
  }
  return out;
}

// Module 1 Actions
function createProcess() {
  const name = document.getElementById("procName").value.trim() || "P1";
  const burst = document.getElementById("procBurst").value.trim() || "5";
  const priority = document.getElementById("procPriority").value.trim() || "1";
  const arrival = document.getElementById("procArrival").value.trim() || "0";

  runCommand(`create ${name} ${burst} ${priority} ${arrival}`);

  const match = name.match(/^P(\d+)$/i);
  if (match) {
    const nextNum = parseInt(match[1]) + 1;
    document.getElementById("procName").value = "P" + nextNum;
  }
}

async function loadSampleProcesses() {
  appendToTerminal("Loading 5 textbook sample processes (P1 to P5)...", false, true);
  await runCommand("clear_all");
  await sleep(150);
  await runCommand("create P1 7 3 0");
  await sleep(150);
  await runCommand("create P2 4 1 2");
  await sleep(150);
  await runCommand("create P3 1 4 4");
  await sleep(150);
  await runCommand("create P4 4 2 5");
  await sleep(150);
  await runCommand("create P5 3 5 6");
  await sleep(150);
  await runCommand("list");
}

function killProcess() {
  const pid = document.getElementById("killPid").value;
  runCommand(`kill ${pid}`);
}

// Module 2 Actions
function runRR() {
  const q = document.getElementById("rrQuantum").value.trim() || "2";
  runCommand(`schedule rr ${q}`);
}

// Module 3 Actions
function allocMemory() {
  const name = document.getElementById("memProcName").value.trim() || "P1";
  const size = document.getElementById("memSize").value.trim() || "128";
  const policy = document.getElementById("memPolicy").value;
  runCommand(`mem_alloc ${name} ${size} ${policy}`);
}

function freeMemory() {
  const name = document.getElementById("memProcName").value.trim() || "P1";
  runCommand(`mem_free ${name}`);
}

// Module 4 Actions
async function runBankers() {
  appendToTerminal("Executing Banker's Algorithm deadlock safety check...", false, true);
  setStatus("Computing Banker's...", true);

  if (useLocalServer) {
    try {
      const res = await fetch("/api/bankers", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({}),
      });
      if (res.ok) {
        const data = await res.json();
        if (data.output) {
          appendToTerminal(data.output);
          setStatus("System Ready", false);
          return;
        }
      }
    } catch {}
  }

  const out = StandaloneOS.bankers();
  appendToTerminal(out);
  setStatus("System Ready", false);
}

// Module 5 Actions
function runPage(algo) {
  const frames = document.getElementById("pageFrames").value.trim() || "3";
  const ref = document.getElementById("pageRef").value.trim() || "7,0,1,2,0,3,0,4,2,3,0,3";
  runCommand(`page ${algo} ${frames} ${ref}`);
}

// Module 6 Actions
function runDisk(algo) {
  const head = document.getElementById("diskHead").value.trim() || "53";
  const requests = document.getElementById("diskRequests").value.trim() || "98,183,37,122,14,124,65,67";
  runCommand(`disk ${algo} ${head} ${requests}`);
}

// ============================================================================
// STEP-BY-STEP FULL DEMO RUNNER (One by One)
// ============================================================================

const demoSteps = [
  { label: "Initialize: Resetting process table", cmd: "clear_all" },
  { label: "Initialize: Resetting 1024 MB memory arena", cmd: "mem_reset" },
  { label: "Creating Process P1 (Burst=7, Priority=3, Arrival=0)", cmd: "create P1 7 3 0" },
  { label: "Creating Process P2 (Burst=4, Priority=1, Arrival=2)", cmd: "create P2 4 1 2" },
  { label: "Creating Process P3 (Burst=1, Priority=4, Arrival=4)", cmd: "create P3 1 4 4" },
  { label: "Creating Process P4 (Burst=4, Priority=2, Arrival=5)", cmd: "create P4 4 2 5" },
  { label: "Creating Process P5 (Burst=3, Priority=5, Arrival=6)", cmd: "create P5 3 5 6" },
  { label: "Listing all processes in Process Table (PCB)", cmd: "list" },
  { label: "Running FCFS (First-Come, First-Served) CPU Scheduling", cmd: "schedule fcfs" },
  { label: "Running SJF (Shortest Job First) CPU Scheduling", cmd: "schedule sjf" },
  { label: "Running Round Robin Scheduling (Time Quantum = 2)", cmd: "schedule rr 2" },
  { label: "Running Priority Scheduling (Non-Preemptive)", cmd: "schedule priority" },
  { label: "Running Multilevel Feedback Queue (MLFQ - 3 Levels)", cmd: "schedule mlfq" },
  { label: "Side-by-Side Comparison of All 5 CPU Scheduling Algorithms", cmd: "schedule compare" },
  { label: "Memory Allocation: P1 requests 128 MB using First Fit (ff)", cmd: "mem_alloc P1 128 ff" },
  { label: "Memory Allocation: P2 requests 256 MB using Best Fit (bf)", cmd: "mem_alloc P2 256 bf" },
  { label: "Memory Allocation: P3 requests 64 MB using Worst Fit (wf)", cmd: "mem_alloc P3 64 wf" },
  { label: "Displaying 1024 MB Memory Allocation Map", cmd: "mem_status" },
  { label: "Freeing memory allocated to P1 & merging adjacent free blocks", cmd: "mem_free P1" },
  { label: "Re-inspecting Memory Map after coalescing free space", cmd: "mem_status" },
  { label: "Synchronization: Simulating Producer-Consumer with Semaphores", cmd: "producer_consumer" },
  { label: "Deadlock Avoidance: Running Banker's Safety Algorithm", action: "bankers" },
  { label: "Virtual Memory: Comparing FIFO vs LRU vs Optimal Page Replacement", cmd: "page compare 3 7,0,1,2,0,3,0,4,2,3,0,3" },
  { label: "Disk Scheduling: Comparing FCFS vs SSTF vs SCAN vs C-SCAN (Head=53)", cmd: "disk compare 53 98,183,37,122,14,124,65,67" },
];

async function toggleFullDemo() {
  if (isDemoRunning) {
    isDemoRunning = false;
    fullDemoBtn.innerText = "⚡ Run Full Demo";
    fullDemoBtn.className = "btn btn-primary btn-sm";
    setStatus("Demo Stopped by User", false);
    appendToTerminal('<span class="ansi-yellow">⏹️ Full Demo stopped by user.</span>');
    return;
  }

  isDemoRunning = true;
  fullDemoBtn.innerText = "⏹️ Stop Demo";
  fullDemoBtn.className = "btn btn-danger btn-sm";

  appendToTerminal('<span class="ansi-cyan">========================================================================\n🚀 Starting Step-by-Step MiniOS Full OS Demonstration (24 Steps)\n   Commands will run one by one with live updates...\n========================================================================</span>');

  for (let i = 0; i < demoSteps.length; i++) {
    if (!isDemoRunning) break;

    const step = demoSteps[i];
    const stepNum = i + 1;
    const progressText = `Step ${stepNum}/${demoSteps.length}: ${step.label}`;

    setStatus(`Running: ${progressText}`, true);
    terminalTitle.innerText = `[Demo Step ${stepNum}/24] ${step.label}`;
    appendToTerminal(`[Step ${stepNum} of ${demoSteps.length}] ${step.label}`, false, true);

    if (step.action === "bankers") {
      await runBankers();
    } else {
      await runCommand(step.cmd);
    }

    if (isDemoRunning && i < demoSteps.length - 1) {
      await sleep(1000);
    }
  }

  if (isDemoRunning) {
    appendToTerminal('<span class="ansi-green">========================================================================\n🎉 Full Step-by-Step OS Demo Completed Successfully!\n   All 24 operations executed across Processes, CPU, Memory, Sync, Paging & Disk.\n========================================================================</span>');
    setStatus("Demo Completed", false);
  }

  isDemoRunning = false;
  fullDemoBtn.innerText = "⚡ Run Full Demo";
  fullDemoBtn.className = "btn btn-primary btn-sm";
  terminalTitle.innerText = "minios-session [interactive output]";
}

async function resetSession() {
  if (isDemoRunning) {
    isDemoRunning = false;
    fullDemoBtn.innerText = "⚡ Run Full Demo";
    fullDemoBtn.className = "btn btn-primary btn-sm";
  }
  setStatus("Restarting MiniOS...", true);

  if (useLocalServer) {
    try {
      await fetch("/api/reset", { method: "POST" });
    } catch {}
  }
  StandaloneOS.reset();
  appendToTerminal('<span class="ansi-yellow">🔄 MiniOS process session restarted cleanly.</span>');
  setStatus("System Ready", false);
}

function clearTerminal() {
  terminal.innerHTML = "";
}

function copyTerminalText() {
  navigator.clipboard.writeText(terminal.innerText).then(() => {
    setStatus("Copied to clipboard!");
    setTimeout(() => setStatus("System Ready"), 2000);
  });
}

function handleCliSend() {
  const cmd = cliInput.value.trim();
  if (!cmd) return;

  cmdHistory.push(cmd);
  historyIndex = cmdHistory.length;

  runCommand(cmd);
  cliInput.value = "";
}

cliInput.addEventListener("keydown", (e) => {
  if (e.key === "Enter") {
    handleCliSend();
  } else if (e.key === "ArrowUp") {
    if (cmdHistory.length > 0 && historyIndex > 0) {
      historyIndex--;
      cliInput.value = cmdHistory[historyIndex];
    }
  } else if (e.key === "ArrowDown") {
    if (historyIndex < cmdHistory.length - 1) {
      historyIndex++;
      cliInput.value = cmdHistory[historyIndex];
    } else {
      historyIndex = cmdHistory.length;
      cliInput.value = "";
    }
  }
});
