#include "file.h"

#include <stdio.h>

#define BUFFER_SIZE 4096

int main(int argc,char *argv[]){
    if(argc != 3){
        printf("Usage: %s <source> <destination>\n",argv[0]);
        return 1;
    }
    const char *source_path = argv[1];
    const char *destination_path = argv[2];
    int source_fd = open_for_read(source_path);
    if(source_fd == -1){
        return 1;
    }
    int destination_fd = open_for_write(destination_path);
    if(destination_fd == -1){
        close_file(source_fd);
        return 1;
    }
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while((bytes_read = read_file(source_fd,buffer,BUFFER_SIZE)) > 0){
        ssize_t bytes_written = write_file(destination_fd,buffer,bytes_read);
        if(bytes_written == -1){
            close_file(source_fd);
            close_file(destination_fd);
            return 1;
        }
        if(bytes_written != bytes_read){
            printf("Error: incomplete write\n");
            close_file(source_fd);
            close_file(destination_fd);
            return 1;
        }
    }
    if(bytes_read == -1){
        close_file(source_fd);
        close_file(destination_fd);
        return 1;
    }
    close_file(source_fd);
    close_file(destination_fd);
    printf("File copied successfully.\n");
    return 0;
}