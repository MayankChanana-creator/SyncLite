#include "block.h"
#include "file.h"


size_t build_block_list(int fd,Block *blocks,size_t max_blocks){
    unsigned char buffer[BLOCK_SIZE];
    size_t block_count = 0;
    ssize_t bytes_read;
    while(block_count < max_blocks){
        bytes_read = read_file(fd,buffer,BLOCK_SIZE);
        if(bytes_read <= 0){
            break;
        }
        blocks[block_count].index = block_count;
        blocks[block_count].size = bytes_read;
        blocks[block_count].checksum = calculate_checksum(buffer,bytes_read);
        block_count++;
    }
    return block_count;
}
int find_matching_block(uint32_t checksum,const Block *blocks,size_t block_count){
    for(size_t i = 0;i < block_count;i++){
        if(blocks[i].checksum == checksum){
            return (int)i;
        }
    }
    return -1;
}
uint32_t calculate_checksum(const unsigned char *data,size_t size){
    uint32_t checksum = 0;
    for(size_t i = 0;i < size;i++){
        checksum += data[i];
    }
    return checksum;
}