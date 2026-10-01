#include "rolling_checksum.h"
#include <stdio.h>

int main(void){
    const unsigned char data[] = "ABCDX";
    uint32_t first = rolling_checksum_initial(data,4);
    if(first != 266){
        printf("FAIL: expected 266, got %u\n",first);
        return 1;
    }
    uint32_t second = rolling_checksum_next(first,'A','X');
    if(second != 289){
        printf("FAIL: expected 289, got %u\n",second);
        return 1;
    }
    printf("PASS: rolling checksum tests\n");
    return 0;
}