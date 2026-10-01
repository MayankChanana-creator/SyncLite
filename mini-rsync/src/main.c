#include "file.h"
#include "block.h"
#include "rolling_checksum.h"
#include "sync.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_BLOCKS 100
#define MAX_OPERATIONS 100

static unsigned char *read_entire_file(const char *path,size_t *file_size){
    int fd = open_for_read(path);

    if (fd == -1){
        return NULL;
    }

    size_t capacity = 1024;
    size_t size = 0;

    unsigned char *buffer = malloc(capacity);

    if(buffer == NULL){
        close_file(fd);
        return NULL;
    }

    while(1){
        if(size == capacity){
            capacity *= 2;

            unsigned char *new_buffer = realloc(buffer, capacity);

            if(new_buffer == NULL){
                free(buffer);
                close_file(fd);
                return NULL;
            }

            buffer = new_buffer;
        }

        ssize_t bytes_read = read_file(fd,&buffer[size],capacity - size);

        if(bytes_read < 0){
            free(buffer);
            close_file(fd);
            return NULL;
        }

        if(bytes_read == 0){
            break;
        }

        size += (size_t)bytes_read;
    }

    close_file(fd);

    *file_size = size;

    return buffer;
}

int main(int argc, char *argv[]){
    if (argc != 3){
        printf("Usage: %s <source> <destination>\n",argv[0]);
        return 1;
    }
    size_t source_size = 0;

    unsigned char *source = read_entire_file(argv[1],&source_size);

    if (source == NULL){
        fprintf(stderr,"Failed to read source file\n");
        return 1;
    }
    size_t destination_size = 0;

    unsigned char *destination = read_entire_file(argv[2],&destination_size);

    if(destination == NULL){
        fprintf(stderr,"Failed to read destination file\n");
        free(source);
        return 1;
    }

    SyncOperation operations[MAX_OPERATIONS];

    size_t operation_count = build_sync_operations(source,source_size,destination,destination_size,operations,MAX_OPERATIONS);

    printf("Generated %zu synchronization operations\n",operation_count);

    int destination_fd = open_for_read(argv[2]);

    if(destination_fd == -1){
        free(source);
        free(destination);

        return 1;
    }

    int output_fd = open_for_write("sync_output.tmp");

    if(output_fd == -1){
        close_file(destination_fd);

        free(source);
        free(destination);

        return 1;
    }

    int result = apply_sync_operations(destination_fd,output_fd,operations,operation_count);

    close_file(destination_fd);
    close_file(output_fd);

    if(result != 0){
        fprintf(stderr,"Failed to apply synchronization operations\n");

        for(size_t i = 0;i < operation_count;i++){
            free_operation(&operations[i]);
        }

        free(source);
        free(destination);

        return 1;
    }

    int new_destination_fd = open_for_read("sync_output.tmp");

    if(new_destination_fd == -1){
        for(size_t i = 0;i < operation_count;i++){
            free_operation(&operations[i]);
        }

        free(source);
        free(destination);

        return 1;
    }

    int final_destination_fd = open_for_write(argv[2]);

    if(final_destination_fd == -1){
        close_file(new_destination_fd);

        for(size_t i = 0;i < operation_count;i++){
            free_operation(&operations[i]);
        }

        free(source);
        free(destination);

        return 1;
    }

    unsigned char buffer[4096];

    while(1){
        ssize_t bytes_read = read_file(new_destination_fd,buffer,sizeof(buffer));

        if(bytes_read < 0){
            close_file(new_destination_fd);
            close_file(final_destination_fd);

            for(size_t i = 0;i < operation_count;i++){
                free_operation(&operations[i]);
            }

            free(source);
            free(destination);

            return 1;
        }

        if(bytes_read == 0){
            break;
        }

        ssize_t bytes_written = write_file(final_destination_fd,buffer,(size_t)bytes_read);

        if(bytes_written != bytes_read){
            close_file(new_destination_fd);
            close_file(final_destination_fd);

            for(size_t i = 0;i < operation_count;i++){
                free_operation(&operations[i]);
            }

            free(source);
            free(destination);

            return 1;
        }
    }

    close_file(new_destination_fd);
    close_file(final_destination_fd);

    for(size_t i = 0;i < operation_count;i++){
        free_operation(&operations[i]);
    }

    free(source);
    free(destination);

    printf("Synchronization completed\n");

    return 0;
}