#include "block.h"

uint32_t calculate_checksum(const unsigned char *data,size_t size){
    uint32_t checksum = 0;
    for(size_t i = 0;i < size;i++){
        checksum += data[i];
    }
    return checksum;
}