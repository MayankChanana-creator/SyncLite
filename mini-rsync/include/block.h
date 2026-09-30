#ifndef BLOCK_H
#define BLOCK_H

#include <stddef.h>
#include <stdint.h>

#define BLOCK_SIZE 4

typedef struct{
    size_t index;
    size_t size;
    uint32_t checksum;
} Block;
uint32_t calculate_checksum(const unsigned char *data,size_t size);

size_t build_block_list(int fd,Block *blocks,size_t max_blocks);
#endif