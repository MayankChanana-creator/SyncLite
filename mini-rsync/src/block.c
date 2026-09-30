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
uint32_t calculate_checksum(const unsigned char *data,size_t size){
    uint32_t checksum = 0;
    for(size_t i = 0;i < size;i++){
        checksum += data[i];
    }
    return checksum;
}