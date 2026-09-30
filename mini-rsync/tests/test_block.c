#include "block.h"
#include <stdio.h>

int main(void){
	const unsigned char data[] = "ABCE";
	uint32_t checksum = calculate_checksum(data,4);
	if(checksum != 267){
		printf("FAIL: expected 267, got %u\n",checksum);
		return 1;
	}
	printf("PASS: checksum test\n");
	return 0;
}
