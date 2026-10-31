CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/process.c src/scheduler.c src/memory.c src/sync.c src/pager.c src/disk.c
TARGET = minios

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
