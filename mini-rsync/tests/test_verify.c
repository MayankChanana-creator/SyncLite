#include "file.h"
#include "rolling_checksum.h"
#include <stdio.h>
int main(void){
    int fd = open_for_read("tests/destination_shifted.txt");
    if(fd == -1){
        return 1;
    }
    const unsigned char correct[] = "ABCD";
    const unsigned char wrong[] = "ABCE";
    int result = verify_block_at_position(fd,correct,4,2);
    if(result != 1){
        printf("FAIL: correct block was not verified\n");
        close_file(fd);
        return 1;
    }
    result = verify_block_at_position(fd,wrong,4,2);
    if(result != 0){
        printf("FAIL: Incorrect block was verified\n");
        close_file(fd);
        return 1;
    }
    close_file(fd);
    printf("PASS: Block Verification\n");
    return 0;
}