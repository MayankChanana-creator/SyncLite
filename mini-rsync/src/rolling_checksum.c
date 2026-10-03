#include "rolling_checksum.h"
#include "file.h"
#include <stdlib.h>
#include <unistd.h>


/*
 * Calculate the checksum of the first window.
 *
 * The checksum is the sum of all bytes in the window.
 */

uint32_t rolling_checksum_initial(const unsigned char *data,size_t size){
    uint32_t sum = 0;
    for(size_t i = 0;i < size;i++){
        sum += data[i];
    }
    return sum;
}


/*
 * Update the checksum when the window moves by one byte.
 *
 * Instead of recalculating the entire window:
 *
 *     previous checksum
 *     - outgoing byte
 *     + incoming byte
 *
 * This makes scanning cheaper.
 */

uint32_t rolling_checksum_next(uint32_t previous,unsigned char outgoing,unsigned char incoming){
    return previous - outgoing + incoming;
}

/*
* Searches a file using the rolling checksum()
*/
int find_checksum_in_file(int fd,uint32_t target,size_t window_size){
    if(window_size == 0){
        return -1;
    }
    unsigned char *buffer = malloc(window_size);
    if(buffer == NULL){
        return -1;
    }
    ssize_t bytes_read = read_file(fd,buffer,window_size);
    if(bytes_read != (ssize_t)window_size){
        free(buffer);
        return -1;
    }
    uint32_t checksum = rolling_checksum_initial(buffer,window_size);
    size_t position = 0;
    while(1){
        if(checksum == target){
            free(buffer);
            return (int)position;
        }
        unsigned char incoming;
        ssize_t result = read_file(fd,&incoming,1);
        if(result <= 0){
            break;
        }
        checksum = rolling_checksum_next(checksum,buffer[position % window_size],incoming);
        buffer[position % window_size] = incoming;
        position++;
    }
    free(buffer);
    return -1;
}

/*
* Searches for a matching block
*/
int find_matching_block_in_file(int fd,const Block *blocks,size_t block_count,size_t window_size){
    for(size_t i = 0;i < block_count;i++){
        if(lseek(fd,0,SEEK_SET) == -1){
            return -1;
        }
        int position = find_checksum_in_file(fd,blocks[i].checksum,window_size);
        if(position != -1){
            return (int)i;
        }
    }
    return -1;
}

/*
* Compare actual bytes to protect against checksum collisions
*/

int verify_block_at_position(int fd,const unsigned char *data,size_t size,size_t position){
    if(lseek(fd,(off_t)position,SEEK_SET) == -1){
        return 0;
    }
    unsigned char *buffer = malloc(size);
    if(buffer == NULL){
        return 0;
    }
    ssize_t bytes_read = read_file(fd,buffer,size);
    if(bytes_read != (ssize_t)size){
        free(buffer);
        return 0;
    }
    for(size_t i = 0;i < size;i++){
        if(buffer[i] != data[i]){
            free(buffer);
            return 0;
        }
    }
    free(buffer);
    return 1;
}

/*
* Combines rolling search + byte verification
*/

int find_verified_block_in_file(int fd,const unsigned char *data,size_t size){
    if(size == 0){
        return -1;
    }
    unsigned char *buffer = malloc(size);
    if(buffer == NULL){
        return -1;
    }
    ssize_t bytes_read = read_file(fd,buffer,size);
    if(bytes_read != (ssize_t)size){
        free(buffer);
        return -1;
    }
    uint32_t target_checksum = rolling_checksum_initial(data,size);
    uint32_t checksum = rolling_checksum_initial(buffer,size);
    size_t position = 0;
    while(1){
        if(checksum == target_checksum){
            int matches = 1;
            for(size_t i = 0;i < size;i++){
                size_t buffer_index = (position + i) % size;
                if(buffer[buffer_index] != data[i]){
                    matches = 0;
                    break;
                }
            }
            if(matches){
                free(buffer);
                return (int)position;
            }
        }
        unsigned char incoming;
        ssize_t result = read_file(fd,&incoming,1);
        if(result <= 0){
            break;
        }
        unsigned char outgoing = buffer[position % size];
        checksum = rolling_checksum_next(checksum,outgoing,incoming);
        buffer[position % size] = incoming;
        position++;
    }
    free(buffer);
    return -1;
}