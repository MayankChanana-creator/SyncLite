#ifndef SYNC_H
#define SYNC_H
#include <stddef.h>
typedef enum{
    OP_COPY,
    OP_INSERT,
} OperationType;

typedef struct{
    OperationType type;
    size_t position;
    size_t size;
    unsigned char *data;
} SyncOperation;
SyncOperation create_copy_operation(size_t position,size_t size);
SyncOperation create_insert_operation(const unsigned char *data,size_t size);
void free_operation(SyncOperation *operation);
size_t build_sync_operations(const unsigned char *source,size_t source_size,const unsigned char *destination,size_t destination_size,SyncOperation *operations,size_t max_operations);
int apply_sync_operations(int destination_fd,int output_fd,const SyncOperation *operations,size_t operation_count);
#endif