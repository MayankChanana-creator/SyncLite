#include "sync.h"
#include "file.h"
#include <stdio.h>


int main(void){
    SyncOperation copy = create_copy_operation(2, 4);

    if(copy.type != OP_COPY){
        printf("FAIL: copy operation type\n");
        return 1;
    }

    if(copy.position != 2){
        printf("FAIL: copy operation position\n");
        return 1;
    }

    if(copy.size != 4){
        printf("FAIL: copy operation size\n");
        return 1;
    }

    if(copy.data != NULL){
        printf("FAIL: copy operation data should be NULL\n");
        return 1;
    }

    const unsigned char data[] = "XYZ";

    SyncOperation insert = create_insert_operation(data, 3);

    if(insert.type != OP_INSERT){
        printf("FAIL: insert operation type\n");
        return 1;
    }

    if(insert.size != 3){
        printf("FAIL: insert operation size\n");
        return 1;
    }

    if(insert.data == NULL){
        printf("FAIL: insert operation data is NULL\n");
        return 1;
    }

    for(size_t i = 0; i < 3; i++){
        if(insert.data[i] != data[i]){
            printf("FAIL: insert operation data\n");
            return 1;
        }
    }

    const unsigned char shifted_source[] = "ABCDEF";
    const unsigned char shifted_destination[] = "XXABCDEFYY";

    SyncOperation shifted_operations[2];

    size_t shifted_count = build_sync_operations(shifted_source,6,shifted_destination,10,shifted_operations,2);
    if(shifted_count != 2){
        printf("FAIL: expected 2 shifted operations, got %zu\n",shifted_count);
        return 1;
    }

    if(shifted_operations[0].type != OP_COPY || shifted_operations[0].position != 2 || shifted_operations[0].size != 4){
        printf("FAIL: shifted first operation\n");
        return 1;
    }

    if(shifted_operations[1].type != OP_COPY || shifted_operations[1].position != 6 || shifted_operations[1].size != 2){
        printf("FAIL: shifted second operation\n");
        return 1;
    }

    free_operation(&copy);
    free_operation(&insert);

    int destination_fd = open_for_read("tests/sync_destination.txt");

    if(destination_fd == -1){
        return 1;
    }

    int output_fd = open_for_write("tests/sync_output.txt");

    if(output_fd == -1){
        close_file(destination_fd);
        return 1;
    }

    int result = apply_sync_operations(destination_fd,output_fd,shifted_operations,shifted_count);

    close_file(destination_fd);
    close_file(output_fd);

    if(result != 0){
        printf("FAIL: applying sync operations\n");
        return 1;
    }
        int verify_fd =
        open_for_read("tests/sync_output.txt");

    if(verify_fd == -1){
        return 1;
    }

    unsigned char result_buffer[7];

    ssize_t bytes_read = read_file(verify_fd,result_buffer,6);

    close_file(verify_fd);

    if(bytes_read != 6){
        printf("FAIL: expected 6 output bytes, got %zd\n",bytes_read);
        return 1;
    }

    const unsigned char expected[] = "ABCDEF";

    for(size_t i = 0; i < 6; i++){
        if(result_buffer[i] != expected[i]){
            printf("FAIL: output mismatch at position %zu\n",i);
            return 1;
        }
    }
    

    printf("PASS: sync operation application\n");

    printf("PASS: sync operation tests\n");
    
    SyncStats stats;

    calculate_sync_stats(shifted_operations,shifted_count,&stats);

    if(stats.operation_count != shifted_count){
        printf("FAIL: statistics operation count\n");
        return 1;
    }

    if(stats.bytes_copied != 6){
        printf("FAIL: statistics copied bytes\n");
        return 1;
    }

    if(stats.bytes_inserted != 0){
        printf("FAIL: statistics inserted bytes\n");
        return 1;
    }

    if(stats.bytes_transferred != 0){
        printf("FAIL: statistics transferred bytes\n");
        return 1;
    }

    printf("PASS: synchronization statistics\n");
    for(size_t i = 0; i < shifted_count; i++){
        free_operation(&shifted_operations[i]);
    }
    return 0;
}