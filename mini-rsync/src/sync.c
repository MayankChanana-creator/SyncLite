#include "sync.h"
#include "block.h"
#include "rolling_checksum.h"
#include "file.h"
#include <stdlib.h>
#include <unistd.h>


/*
 * Create an operation that copies bytes from an
 * existing position in the destination file.
 *
 * No memory is allocated because the data already
 * exists in the destination.
 */

SyncOperation create_copy_operation(size_t position,size_t size){
    SyncOperation operation;
    operation.type = OP_COPY;
    operation.position = position;
    operation.size = size;
    operation.data = NULL;
    return operation;
}

/*
 * Create an operation that inserts new bytes.
 *
 * The data is copied into dynamically allocated
 * memory because it must remain available until
 * synchronization is applied.
 */


SyncOperation create_insert_operation(const unsigned char *data,size_t size){
    SyncOperation operation;
    operation.type = OP_INSERT;
    operation.position = 0;
    operation.size = size;
    operation.data = malloc(size);

    if(operation.data == NULL){
        operation.size = 0;
        return operation;
    }

    for(size_t i = 0; i < size; i++){
        operation.data[i] = data[i];
    }
    return operation;
}

/*
 * Release memory owned by an INSERT operation.
 *
 * COPY operations have data == NULL, so free(NULL) is safe.
 */

void free_operation(SyncOperation *operation){
    if(operation == NULL){
        return;
    }
    free(operation->data);

    operation->data = NULL;
    operation->size = 0;
}

/*
 * Search the destination for a source block.
 *
 * The search uses two stages:
 *
 * 1. Compare rolling checksums.
 * 2. If the checksum matches, compare every byte.
 *
 * The second step is necessary because different
 * byte sequences can produce the same checksum.
 */

static int find_block_in_destination(const unsigned char *source,size_t source_size,const unsigned char *destination,size_t destination_size){
    if(source_size == 0 || destination_size < source_size){
        return -1;
    }

    uint32_t target_checksum = rolling_checksum_initial(source,source_size);

    uint32_t checksum = rolling_checksum_initial(destination,source_size);

    for(size_t position = 0;position <= destination_size - source_size;position++){
        if(checksum == target_checksum){
            int matches = 1;

            for(size_t i = 0; i < source_size; i++){
                if(destination[position + i] != source[i]){
                    matches = 0;
                    break;
                }
            }
            if(matches){
                return (int)position;
            }
        }

        if(position < destination_size - source_size){
            checksum = rolling_checksum_next(checksum,destination[position],destination[position + source_size]);
        }
    }

    return -1;
}


/*
 * Build the synchronization plan.
 *
 * The source is processed in fixed-size blocks.
 *
 * For every source block:
 *
 *     matching destination block -> COPY
 *
 *     no matching block -> INSERT
 *
 * The resulting operations are later applied to
 * produce the synchronized destination.
 */

size_t build_sync_operations(const unsigned char *source,size_t source_size,const unsigned char *destination,size_t destination_size,SyncOperation *operations,size_t max_operations){
    size_t operation_count = 0;
    size_t source_position = 0;
    while(source_position < source_size && operation_count < max_operations){
        size_t remaining = source_size - source_position;

        size_t block_size = BLOCK_SIZE;

        if(remaining < block_size){
            block_size = remaining;
        }

        int destination_position = find_block_in_destination(&source[source_position],block_size,destination,destination_size);

        if(destination_position != -1){
            operations[operation_count] = create_copy_operation((size_t)destination_position,block_size);
        }
        else{
            operations[operation_count] = create_insert_operation(&source[source_position],block_size);
        }
        operation_count++;
        source_position += block_size;
    }

    return operation_count;
}

/*
 * Apply the synchronization plan.
 *
 * COPY:
 *     Read bytes from the existing destination
 *     at the specified position.
 *
 * INSERT:
 *     Write the stored new bytes directly.
 *
 * The output is written to a separate file rather
 * than modifying the destination in place.
 */


int apply_sync_operations(int destination_fd,int output_fd,const SyncOperation *operations,size_t operation_count){
    unsigned char buffer[BLOCK_SIZE];

    for(size_t i = 0; i < operation_count; i++){
        const SyncOperation *operation = &operations[i];

        if(operation->type == OP_COPY){
            if(lseek(destination_fd,(off_t)operation->position,SEEK_SET) == -1){
                return -1;
            }
            size_t remaining = operation->size;

            while (remaining > 0){
                size_t chunk_size = remaining;

                if(chunk_size > sizeof(buffer)){
                    chunk_size = sizeof(buffer);
                }
                ssize_t bytes_read = read_file(destination_fd,buffer,chunk_size);

                if(bytes_read <= 0){
                    return -1;
                }

                ssize_t bytes_written = write_file(output_fd,buffer,(size_t)bytes_read);

                if(bytes_written != bytes_read){
                    return -1;
                }
                remaining -= (size_t)bytes_read;
            }
        }
        else if(operation->type == OP_INSERT){
            ssize_t bytes_written = write_file(output_fd,operation->data,operation->size);

            if(bytes_written != (ssize_t)operation->size){
                return -1;
            }
        }
        else{
            return -1;
        }
    }

    return 0;
}

/*
 * Calculate statistics for the synchronization plan.
 *
 * Bytes copied represent data reused from the
 * destination.
 *
 * Bytes inserted represent new data that must be
 * transferred.
 *
 * Bytes transferred therefore counts only INSERT
 * data in the current implementation.
 */

 
void calculate_sync_stats(const SyncOperation *operations,size_t operation_count,SyncStats *stats){
    if(stats == NULL){
        return;
    }
    stats->operation_count = operation_count;
    stats->bytes_copied = 0;
    stats->bytes_inserted = 0;
    stats->bytes_transferred = 0;
    for(size_t i = 0;i < operation_count;i++){
        if(operations[i].type == OP_COPY){
            stats->bytes_copied += operations[i].size;
        }
        else if(operations[i].type == OP_INSERT){
            stats->bytes_inserted += operations[i].size;
            stats->bytes_transferred += operations[i].size;
        }
    }
}