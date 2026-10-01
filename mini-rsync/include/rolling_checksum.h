#ifndef ROLLING_CHECKSUM_H

#define ROLLING_CHECKSUM_H
#include <stddef.h>
#include <stdint.h>
#include "block.h"

typedef struct{
    uint32_t sum;
} RollingChecksum;

uint32_t rolling_checksum_initial(const unsigned char *data, size_t size);

uint32_t rolling_checksum_next(uint32_t previous,unsigned char outgoing,unsigned char incoming);

int find_checksum_in_file(int fd,uint32_t target,size_t window_size);
int find_matching_block_in_file(int fd,const Block *blocks,size_t block_count,size_t window_size);
int verify_block_at_position(int fd,const unsigned char *data,size_t size,size_t position);
int find_verified_block_in_file(int fd,const unsigned char *data,size_t size);
#endif
