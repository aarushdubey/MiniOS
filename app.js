// MiniOS Simulator Dashboard Client-Side Logic

const terminal = document.getElementById("terminalOutput");
const cliInput = document.getElementById("cliInput");
const statusDot = document.querySelector(".status-dot");
const statusText = document.getElementById("statusText");

// Command history
let cmdHistory = [];
let historyIndex = -1;

// ANSI escape sequence to HTML converter
function ansiToHtml(text) {
  if (!text) return "";
  
  // Escape HTML characters first
  let safe = text
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;");

  // ANSI color replacements
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

function appendToTerminal(rawText, isUserInput = false) {
  if (isUserInput) {
    const promptLine = document.createElement("div");
    promptLine.innerHTML = `<span style="color:#06b6d4;font-weight:600;">minios&gt;</span> <span style="color:#f8fafc;">${rawText}</span>`;
    terminal.appendChild(promptLine);
  } else {
    const responseBlock = document.createElement("div");
    responseBlock.innerHTML = ansiToHtml(rawText);
    terminal.appendChild(responseBlock);
  }
  
  // Auto scroll to bottom
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

// Send a single command to MiniOS
async function runCommand(cmd, waitSeconds = 0.25) {
  if (!cmd || !cmd.trim()) return;
  const cleanCmd = cmd.trim();
  
  appendToTerminal(cleanCmd, true);
  setStatus("Executing...", true);

  try {
    const res = await fetch("/api/cmd", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ command: cleanCmd, wait: waitSeconds })
    });
    const data = await res.json();
    if (data.output) {
      appendToTerminal(data.output);
    }
  } catch (err) {
    appendToTerminal(`<span class="ansi-red">Error communicating with server: ${err.message}</span>`);
  } finally {
    setStatus("System Ready", false);
  }
}

// Send a batch of commands to MiniOS
async function runBatch(commands) {
  if (!commands || commands.length === 0) return;
  
  setStatus("Running batch...", true);
  try {
    const res = await fetch("/api/batch", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ commands })
    });
    const data = await res.json();
    if (data.output) {
      appendToTerminal(data.output);
    }
  } catch (err) {
    appendToTerminal(`<span class="ansi-red">Error running batch: ${err.message}</span>`);
  } finally {
    setStatus("System Ready", false);
  }
}

// Module 1: Process Management Actions
function createProcess() {
  const name = document.getElementById("procName").value.trim() || "P1";
  const burst = document.getElementById("procBurst").value.trim() || "5";
  const priority = document.getElementById("procPriority").value.trim() || "1";
  const arrival = document.getElementById("procArrival").value.trim() || "0";

  runCommand(`create ${name} ${burst} ${priority} ${arrival}`);

  // Auto-increment name for convenience
  const match = name.match(/^P(\d+)$/i);
  if (match) {
    const nextNum = parseInt(match[1]) + 1;
    document.getElementById("procName").value = "P" + nextNum;
  }
}

function loadSampleProcesses() {
  appendToTerminal("Loading 5 textbook sample processes (P1 to P5)...", true);
  runBatch([
    "clear_all",
    "create P1 7 3 0",
    "create P2 4 1 2",
    "create P3 1 4 4",
    "create P4 4 2 5",
    "create P5 3 5 6",
    "list"
  ]);
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
  appendToTerminal("Executing Banker's Algorithm with standard 5-process allocation matrix...", true);
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
    setStatus("System Ready", false);
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

// Top Bar Actions
async function runQuickDemo() {
  appendToTerminal("⚡ Running complete end-to-end OS feature demonstration...", true);
  await runBatch([
    "clear_all",
    "create P1 7 3 0",
    "create P2 4 1 2",
    "create P3 1 4 4",
    "create P4 4 2 5",
    "create P5 3 5 6",
    "list",
    "schedule compare",
    "mem_alloc P1 128 ff",
    "mem_alloc P2 256 bf",
    "mem_status",
    "mem_free P1",
    "mem_status",
    "producer_consumer",
    "page compare 3 7,0,1,2,0,3,0,4,2,3,0,3",
    "disk compare 53 98,183,37,122,14,124,65,67"
  ]);
}

async function resetSession() {
  setStatus("Restarting MiniOS...", true);
  try {
    const res = await fetch("/api/reset", { method: "POST" });
    const data = await res.json();
    appendToTerminal("<span class=\"ansi-yellow\">🔄 MiniOS process session restarted.</span>");
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
