#include "file.h"

#include "rolling_checksum.h"
#include <stdio.h>
int main(void){
    int fd = open_for_read("tests/rolling_source.txt");
    if(fd == -1){
        printf("FAIL: could not open file\n");
        return 1;
    }
    int position = find_checksum_in_file(fd,266,4);
    close_file(fd);
    if(position != 4){
        printf("FAIL: expected position 4, got %d\n",position);
        return 1;
    }
    printf("PASS: rolling file scan\n");
    return 0;
}