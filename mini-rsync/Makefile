CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -Iinclude

TARGET = mini-rsync.exe

SRC = src/main.c src/file.c src/block.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	del /Q $(TARGET).exe 2>NUL