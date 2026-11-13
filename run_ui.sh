#!/bin/bash
# One-click launcher for MiniOS Web UI
cd "$(dirname "$0")"

# Ensure binary is built
make

# Open default browser
python3 -m webbrowser "http://127.0.0.1:5050" 2>/dev/null &

# Launch Python backend server
python3 ui/app.py

