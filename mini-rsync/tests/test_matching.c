#include "block.h"
#include <stdio.h>

int main(void){
    Block blocks[3] = {{0,4,266},{1,4,352},{2,4,300}};
    int match = find_matching_block(352,blocks,3);
    if(match != 1){
        printf("FAIL: expected block 1, got %d\n",match);
        return 1;
    }
    match = find_matching_block(999,blocks,3);
    if(match != -1){
        printf("FAIL: expected -1, got %d\n",match);
        return 1;
    }
    match = find_matching_block(266,blocks,3);
    if(match != 0){
        printf("FAIL: expected block 0, got %d\n",match);
        return 1;
    }
    match = find_matching_block(266,blocks,0);
    if(match != -1){
        printf("FAIL: expected -1 for empty list, got %d\n",match);
        return 1;
    }
    printf("PASS: block matching tests\n");
    return 0;
}