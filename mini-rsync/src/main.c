#include "file.h"
#include "block.h"
#include "rolling_checksum.h"

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
            printf("Source Block %zu -> ALIGNED MATCH -> Destination Block %d\n",source_index,match);
        }
        else{
            int destination_fd = open_for_read(argv[2]);
            if(destination_fd == -1){
                close_file(source_fd);
                return 1;
            }
            int rolling_match = find_checksum_in_file(destination_fd,checksum,bytes_read);
            close_file(destination_fd);
            if(rolling_match != -1){
                int verify_fd = open_for_read(argv[2]);
                if(verify_fd == -1){
                    close_file(source_fd);
                    return 1;
                }
                int verified = verify_block_at_position(verify_fd,buffer,bytes_read,(size_t)rolling_match);
                close_file(verify_fd);
                if(verified){
                    printf("Source Block %zu -> SHIFTED MATCH -> Destination Position %d\n",source_index,rolling_match);
                }
                else{
                    printf("Source Block %zu -> CHECKSUM COLLISION\n",source_index);
                }
            }
            else{
                printf("Source Block %zu -> NO MATCH\n",source_index);
            }
        }
        source_index++;
    }
    if(bytes_read == -1){
        close_file(source_fd);
        return 1;
    }
    close_file(source_fd);
    return 0;
}