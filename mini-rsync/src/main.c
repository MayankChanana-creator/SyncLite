#include "file.h"
#include "block.h"

#include <stdio.h>

#define MAX_BLOCKS 100

int main(int argc,char *argv[]){
    if(argc != 3){
        printf("Usage: %s <source> <destination>\n",argv[0]);
        return 1;
    }
    int destination_fd = open_for_read(argv[2]);
    if(destination_fd == -1){
        return 1;
    }
    Block destination_blocks[MAX_BLOCKS];
    size_t destination_count = build_block_list(destination_fd,destination_blocks,MAX_BLOCKS);
    close_file(destination_fd);
    int source_fd = open_for_read(argv[1]);
    if(source_fd == -1){
        return 1;
    }
    unsigned char buffer[BLOCK_SIZE];
    size_t source_index = 0;
    ssize_t bytes_read;
    while((bytes_read = read_file(source_fd,buffer,BLOCK_SIZE)) > 0){
        uint32_t checksum = calculate_checksum(buffer,bytes_read);
        int match = find_matching_block(checksum,destination_blocks,destination_count);
        if(match != -1){
            printf("Source Block %zu -> MATCH -> Destination Block %d\n",source_index,match);
        }
        else{
            printf("Source Block %zu -> NO MATCH\n",source_index);
        }
        source_index++;
    }
    close_file(source_fd);
    return 0;
}