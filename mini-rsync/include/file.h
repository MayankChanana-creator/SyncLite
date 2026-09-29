#ifndef FILE_H
#define FILE_H

#include <stddef.h>
#include <sys/types.h>

int open_for_read(const char *path);
int open_for_write(const char *path);

ssize_t read_file(int fd, void *buffer,size_t size);

ssize_t write_file(int fd,const void *buffer,size_t size);

void close_file(int fd);

#endif