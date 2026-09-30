#include "file.h"
#include "block.h"

#include <stdio.h>

#define MAX_BLOCKS 100

int main(int argc,char *argv[]){
    if(argc != 2){
        printf("Usage: %s <file>\n",argv[0]);
        return 1;
    }
    int fd = open_for_read(argv[1]);
    if(fd == -1){
        return 1;
    }
    Block blocks[MAX_BLOCKS];
    size_t block_count = build_block_list(fd,blocks,MAX_BLOCKS);
    close_file(fd);
    for(size_t i = 0;i < block_count;i++){
        printf("Block %zu: size=%zu checksum=%u\n",blocks[i].index,blocks[i].size,blocks[i].checksum);
    }
    return 0;
}