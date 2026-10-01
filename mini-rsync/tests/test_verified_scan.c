#include "file.h"
#include "rolling_checksum.h"
#include <stdio.h>
int main(void){
    int fd = open_for_read("tests/collision_destination.txt");
    if(fd == -1){
        return 1;
    }
    const unsigned char target[] = "ABCD";
    int position = find_verified_block_in_file(fd,target,4);
    close_file(fd);
    if(position != -1){
        printf("FAIL: expected no verified match, got position %d\n",position);
        return 1;
    }
    printf("PASS: collision rejected\n");
    return 0;
}