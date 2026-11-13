CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/process.c src/scheduler.c src/memory.c src/sync.c src/pager.c src/disk.c
TARGET = minios

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

ui: $(TARGET)
	@echo "Starting MiniOS Web UI..."
	@python3 -m webbrowser "http://127.0.0.1:5050" 2>/dev/null &
	@python3 ui/app.py

.PHONY: all clean ui
