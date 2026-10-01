#include "block.h"
#include "file.h"
#include "rolling_checksum.h"
#include <stdio.h>

int main(void){
    int fd = open_for_read("tests/rolling_destination.txt");
    if(fd == -1){
        return 1;
    }
    Block blocks[1] = {{0,4,266}};
    int match = find_matching_block_in_file(fd,blocks,1,4);
    close_file(fd);
    if(match != 0){
        printf("FAIL: expected block 0, got %d\n",match);
        return 1;
    }
    printf("PASS: shifted block matching\n");
    return 0;
}