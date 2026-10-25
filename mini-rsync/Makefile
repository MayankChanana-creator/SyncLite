
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Iinclude

TARGET = mini-rsync
TEST_TARGET = test_sync

SRC = \
	src/main.c \
	src/file.c \
	src/block.c \
	src/rolling_checksum.c \
	src/sync.c

TEST_SRC = \
	tests/test_sync.c \
	src/file.c \
	src/block.c \
	src/rolling_checksum.c \
	src/sync.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC)
	$(CC) $(CFLAGS) $(TEST_SRC) -o $(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) mini-rsync.exe mini-rsync.exe.exe

