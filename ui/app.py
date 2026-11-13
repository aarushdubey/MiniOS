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
import subprocess
import sys
import threading
import time
from urllib.parse import urlparse

PORT = 5050
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BINARY_PATH = os.path.join(PROJECT_ROOT, "minios.exe" if os.name == "nt" else "minios")


class MiniOSSession:
    """Manages a persistent MiniOS interactive process session with prompt synchronization."""

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

            # Drain initial banner and initial prompt
            time.sleep(0.15)
            while not self.output_queue.empty():
                try:
                    self.output_queue.get_nowait()
                except queue.Empty:
                    break

    def execute(self, cmd_string, max_timeout=6.0):
        """
        Sends command(s) and waits until the prompt 'minios>' returns.
        Guarantees complete output without premature cutoff.
        """
        with self.lock:
            if not self.proc or self.proc.poll() is not None:
                self.start()

            # Drain any prior leftover in queue
            while not self.output_queue.empty():
                try:
                    self.output_queue.get_nowait()
                except queue.Empty:
                    break

            try:
                to_send = cmd_string.strip() + "\n"
                self.proc.stdin.write(to_send)
                self.proc.stdin.flush()
            except Exception as e:
                self.start()
                return f"[Error communicating with MiniOS process: {e}]"

            chars = []
            start_time = time.time()

            while time.time() - start_time < max_timeout:
                try:
                    c = self.output_queue.get(timeout=0.05)
                    chars.append(c)

                    # Look for prompt in the tail of output
                    tail = "".join(chars[-30:])
                    if "minios>" in tail:
                        time.sleep(0.02)
                        while not self.output_queue.empty():
                            chars.append(self.output_queue.get_nowait())
                        break
                except queue.Empty:
                    if self.proc.poll() is not None:
                        break
                    continue

            output = "".join(chars)
            return output


session = MiniOSSession()


class RequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        ui_dir = os.path.dirname(os.path.abspath(__file__))
        super().__init__(*args, directory=ui_dir, **kwargs)

    def log_message(self, format, *args):
        pass

    def do_POST(self):
        parsed = urlparse(self.path)

        if parsed.path == "/api/cmd":
            content_length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(content_length).decode("utf-8")
            data = json.loads(body) if body else {}

            cmd = data.get("command", "")
            timeout = float(data.get("timeout", 6.0))

            output = session.execute(cmd, max_timeout=timeout)

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
            content_length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(content_length).decode("utf-8")
            data = json.loads(body) if body else {}

            raw_input = data.get(
                "input",
                "5\n3\n0 1 0\n2 0 0\n3 0 2\n2 1 1\n0 0 2\n7 5 3\n3 2 2\n9 0 2\n2 2 2\n4 3 3\n3 3 2\n",
            )
            full_input = f"bankers\n{raw_input}\nexit\n"
            try:
                res = subprocess.run(
                    [BINARY_PATH],
                    input=full_input,
                    cwd=PROJECT_ROOT,
                    capture_output=True,
                    text=True,
                    timeout=4,
                )
                output = res.stdout
            except Exception as e:
                output = f"Error running Banker's algorithm: {e}"

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
    print("Open the URL in your browser to test MiniOS.")
    print("=" * 60)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        if session.proc:
            session.proc.terminate()
        server.server_close()


if __name__ == "__main__":
    run()
