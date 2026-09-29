#include "file.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int open_for_read(const char *path){
    int fd = open(path,O_RDONLY);
    if(fd == -1){
        perror("Error opening file for reading");
    }
    return fd;
}
int open_for_write(const char *path){
    int fd = open(path,O_WRONLY | O_CREAT | O_TRUNC,0644);
    if(fd == -1){
        perror("Error opening file for writing");
    }
    return fd;
}

ssize_t read_file(int fd,void *buffer,size_t size){
    ssize_t bytes_read = read(fd,buffer,size);
    if(bytes_read == -1){
        perror("Error reading file");
    }
    return bytes_read;
}

ssize_t write_file(int fd,const void *buffer,size_t size){
    ssize_t bytes_written = write(fd,buffer,size);
    if(bytes_written == -1){
        perror("Error writing file");
    }
    return bytes_written;
}

void close_file(int fd){
    if(close(fd) == -1){
        perror("Error closing file");
    }
}
