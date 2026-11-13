// MiniOS Simulator Dashboard Client-Side Logic

const terminal = document.getElementById("terminalOutput");
const cliInput = document.getElementById("cliInput");
const statusDot = document.querySelector(".status-dot");
const statusText = document.getElementById("statusText");
const fullDemoBtn = document.getElementById("fullDemoBtn");
const terminalTitle = document.getElementById("terminalTitle");

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

// Sleep helper
function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

// Send a single command to MiniOS
async function runCommand(cmd, timeoutSeconds = 6.0) {
  if (!cmd || !cmd.trim()) return;
  const cleanCmd = cmd.trim();
  
  appendToTerminal(cleanCmd, true);
  setStatus("Executing...", true);

  try {
    const res = await fetch("/api/cmd", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ command: cleanCmd, timeout: timeoutSeconds })
    });
    const data = await res.json();
    if (data.output) {
      appendToTerminal(data.output);
    }
    return data.output;
  } catch (err) {
    appendToTerminal(`<span class="ansi-red">Error communicating with server: ${err.message}</span>`);
  } finally {
    if (!isDemoRunning) {
      setStatus("System Ready", false);
    }
  }
}

// Toggle card collapse / expand
function toggleCard(header) {
  const card = header.closest(".card");
  if (card) {
    card.classList.toggle("collapsed");
  }
}

// Module 1: Process Management Actions
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
  await sleep(200);
  await runCommand("create P1 7 3 0");
  await sleep(200);
  await runCommand("create P2 4 1 2");
  await sleep(200);
  await runCommand("create P3 1 4 4");
  await sleep(200);
  await runCommand("create P4 4 2 5");
  await sleep(200);
  await runCommand("create P5 3 5 6");
  await sleep(200);
  await runCommand("list");
}

function killProcess() {
  const pid = document.getElementById("killPid").value;
  runCommand(`kill ${pid}`);
}

// Module 2: CPU Scheduling Actions
function runRR() {
  const q = document.getElementById("rrQuantum").value.trim() || "2";
  runCommand(`schedule rr ${q}`);
}

// Module 3: Memory Management Actions
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

// Module 4: Synchronization Actions
async function runBankers() {
  appendToTerminal("Executing Banker's Algorithm deadlock safety check...", false, true);
  setStatus("Computing Banker's...", true);
  try {
    const res = await fetch("/api/bankers", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({})
    });
    const data = await res.json();
    if (data.output) {
      appendToTerminal(data.output);
    }
  } catch (err) {
    appendToTerminal(`<span class="ansi-red">Error running Banker's algorithm: ${err.message}</span>`);
  } finally {
    if (!isDemoRunning) {
      setStatus("System Ready", false);
    }
  }
}

// Module 5: Page Replacement Actions
function runPage(algo) {
  const frames = document.getElementById("pageFrames").value.trim() || "3";
  const ref = document.getElementById("pageRef").value.trim() || "7,0,1,2,0,3,0,4,2,3,0,3";
  runCommand(`page ${algo} ${frames} ${ref}`);
}

// Module 6: Disk Scheduling Actions
function runDisk(algo) {
  const head = document.getElementById("diskHead").value.trim() || "53";
  const requests = document.getElementById("diskRequests").value.trim() || "98,183,37,122,14,124,65,67";
  runCommand(`disk ${algo} ${head} ${requests}`);
}

// ============================================================================
// STEP-BY-STEP FULL DEMO RUNNER (One by One with Live Updates)
// ============================================================================

const demoSteps = [
  { label: "Initialize: Resetting process table & memory arena", cmd: "clear_all" },
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
    // User requested to stop
    isDemoRunning = false;
    fullDemoBtn.innerText = "⚡ Run Full Demo";
    fullDemoBtn.className = "btn btn-primary btn-sm";
    setStatus("Demo Stopped by User", false);
    appendToTerminal("<span class=\"ansi-yellow\">⏹️ Full Demo stopped by user.</span>");
    return;
  }

  isDemoRunning = true;
  fullDemoBtn.innerText = "⏹️ Stop Demo";
  fullDemoBtn.className = "btn btn-danger btn-sm";

  appendToTerminal("<span class=\"ansi-cyan\">========================================================================\n🚀 Starting Step-by-Step MiniOS Full OS Demonstration (24 Steps)\n   Commands will run one by one with live updates...\n========================================================================</span>");

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

    // Friendly 1-second pause between commands so user can watch step by step
    if (isDemoRunning && i < demoSteps.length - 1) {
      await sleep(1000);
    }
  }

  if (isDemoRunning) {
    appendToTerminal("<span class=\"ansi-green\">========================================================================\n🎉 Full Step-by-Step OS Demo Completed Successfully!\n   All 24 operations executed across Processes, CPU, Memory, Sync, Paging & Disk.\n========================================================================</span>");
    setStatus("Demo Completed", false);
  }

  isDemoRunning = false;
  fullDemoBtn.innerText = "⚡ Run Full Demo";
  fullDemoBtn.className = "btn btn-primary btn-sm";
  terminalTitle.innerText = "minios-session [interactive output]";
}

// Reset session
async function resetSession() {
  if (isDemoRunning) {
    isDemoRunning = false;
    fullDemoBtn.innerText = "⚡ Run Full Demo";
    fullDemoBtn.className = "btn btn-primary btn-sm";
  }
  setStatus("Restarting MiniOS...", true);
  try {
    const res = await fetch("/api/reset", { method: "POST" });
    const data = await res.json();
    appendToTerminal("<span class=\"ansi-yellow\">🔄 MiniOS process session restarted cleanly.</span>");
  } catch (err) {
    appendToTerminal(`<span class="ansi-red">Failed to reset: ${err.message}</span>`);
  } finally {
    setStatus("System Ready", false);
  }
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

// CLI Input Bar Handlers
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
