CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -Iinclude

TARGET = mini-rsync

SRC = src/main.c src/file.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	del /Q $(TARGET).exe 2>NUL