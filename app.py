#!/usr/bin/env python3
"""
MiniOS Web UI Server
A lightweight, zero-dependency HTTP server that connects a browser UI to the MiniOS C binary.
Works on macOS, Linux, and Windows.
"""

import http.server
import json
import os
import queue
import select
import subprocess
import sys
import threading
import time
from urllib.parse import urlparse

PORT = 5050
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BINARY_PATH = os.path.join(PROJECT_ROOT, "minios.exe" if os.name == "nt" else "minios")


class MiniOSSession:
    """Manages a persistent MiniOS interactive process session."""

    def __init__(self):
        self.proc = None
        self.lock = threading.Lock()
        self.output_queue = queue.Queue()
        self.reader_thread = None
        self.ensure_binary()
        self.start()

    def ensure_binary(self):
        """Compiles minios if not present."""
        if not os.path.exists(BINARY_PATH):
            print(f"[MiniOS Server] Binary not found at {BINARY_PATH}. Compiling with make...")
            res = subprocess.run(["make"], cwd=PROJECT_ROOT, capture_output=True, text=True)
            if res.returncode != 0:
                print(f"[MiniOS Server] Compilation failed: {res.stderr}")
            else:
                print("[MiniOS Server] Compilation succeeded.")

    def start(self):
        with self.lock:
            if self.proc and self.proc.poll() is None:
                try:
                    self.proc.terminate()
                    self.proc.wait(timeout=1)
                except Exception:
                    self.proc.kill()

            self.proc = subprocess.Popen(
                [BINARY_PATH],
                cwd=PROJECT_ROOT,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=0,
            )
            self.output_queue = queue.Queue()

            def reader():
                while self.proc and self.proc.poll() is None:
                    try:
                        char = self.proc.stdout.read(1)
                        if not char:
                            break
                        self.output_queue.put(char)
                    except Exception:
                        break

            self.reader_thread = threading.Thread(target=reader, daemon=True)
            self.reader_thread.start()

            # Drain initial banner and first prompt
            time.sleep(0.15)
            self._drain()

    def _drain(self, timeout=0.25):
        """Collects any pending characters from the output queue."""
        chars = []
        end_time = time.time() + timeout
        while time.time() < end_time:
            try:
                c = self.output_queue.get(timeout=0.03)
                chars.append(c)
                # If we get chars, give a tiny bit more time for full burst
                end_time = max(end_time, time.time() + 0.05)
            except queue.Empty:
                break
        return "".join(chars)

    def execute(self, cmd_string, wait_seconds=0.3):
        """Sends command(s) and waits for output."""
        with self.lock:
            if not self.proc or self.proc.poll() is not None:
                self.start()

            # Drain prior output
            self._drain(timeout=0.05)

            try:
                # Ensure trailing newline
                to_send = cmd_string.strip() + "\n"
                self.proc.stdin.write(to_send)
                self.proc.stdin.flush()
            except Exception as e:
                self.start()
                return f"[Error writing to MiniOS process: {e}]"

            # Read response
            output = self._drain(timeout=wait_seconds)
            # Remove repeated shell prompt prefix/suffix if desired, but keep full raw text
            return output

    def run_batch(self, commands, delay=0.15):
        """Runs a list of commands in sequence."""
        combined_output = []
        for cmd in commands:
            out = self.execute(cmd, wait_seconds=delay)
            if out:
                combined_output.append(out)
        return "".join(combined_output)


session = MiniOSSession()


class RequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        ui_dir = os.path.dirname(os.path.abspath(__file__))
        super().__init__(*args, directory=ui_dir, **kwargs)

    def log_message(self, format, *args):
        # Concise logging
        pass

    def do_POST(self):
        parsed = urlparse(self.path)

        if parsed.path == "/api/cmd":
            content_length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(content_length).decode("utf-8")
            data = json.loads(body) if body else {}

            cmd = data.get("command", "")
            wait_time = float(data.get("wait", 0.25))

            output = session.execute(cmd, wait_seconds=wait_time)

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps({"output": output, "status": "ok"}).encode("utf-8"))

        elif parsed.path == "/api/batch":
            content_length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(content_length).decode("utf-8")
            data = json.loads(body) if body else {}

            commands = data.get("commands", [])
            output = session.run_batch(commands)

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps({"output": output, "status": "ok"}).encode("utf-8"))

        elif parsed.path == "/api/reset":
            session.start()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps({"status": "reset_complete"}).encode("utf-8"))

        elif parsed.path == "/api/bankers":
            # Banker's algorithm with piped input
            # Standard textbook example
            content_length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(content_length).decode("utf-8")
            data = json.loads(body) if body else {}

            raw_input = data.get(
                "input",
                "5\n3\n0 1 0\n2 0 0\n3 0 2\n2 1 1\n0 0 2\n7 5 3\n3 2 2\n9 0 2\n2 2 2\n4 3 3\n3 3 2\n",
            )
            # Run banker's as a one-shot process to handle all stdin prompts cleanly
            full_input = f"bankers\n{raw_input}\nexit\n"
            try:
                res = subprocess.run(
                    [BINARY_PATH],
                    input=full_input,
                    cwd=PROJECT_ROOT,
                    capture_output=True,
                    text=True,
                    timeout=3,
                )
                output = res.stdout
            except Exception as e:
                output = f"Error running Bankers: {e}"

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps({"output": output, "status": "ok"}).encode("utf-8"))

        else:
            self.send_error(404, "Endpoint not found")

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()


def run():
    server = http.server.ThreadingHTTPServer(("127.0.0.1", PORT), RequestHandler)
    print("=" * 60)
    print(f"🚀 MiniOS Web UI running at: http://127.0.0.1:{PORT}")
    print("Open the URL in any web browser to test MiniOS interactively.")
    print("Press Ctrl+C to stop the server.")
    print("=" * 60)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server...")
        if session.proc:
            session.proc.terminate()
        server.server_close()


if __name__ == "__main__":
    run()
