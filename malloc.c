#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "malloc.h"

// Store how much space in the immediate next block is used
union blocksize{
	size_t as_size_t;
	char as_char[sizeof(size_t)];
};
// When a block is NOT in use, let the default value be zero; when a block is in use, let its first byte be nonzero
uint8_t *page;
void malloc_init(void){
	if(page==NULL)
		page = mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
	*(page+0) = 1;
	union blocksize block;
	block.as_size_t = getpagesize()-1-sizeof(size_t);
	memcpy(page+1, block.as_char, sizeof(size_t));
	return;
}
//Print all blocks within page
void print_blocks(void){
	size_t index = 0;
	printf("sizeof size_t is %u\n", sizeof(size_t));
	while(index < getpagesize()){
		printf("Scanning index %u: ", index);
		if(*(page+index)) printf("Block in use");
		else printf("Block is free");
		index++;
		union blocksize block;
		memcpy(block.as_char, page+index, sizeof(size_t));
		printf(", size of block is %u bits\n", block.as_size_t);
		index+=sizeof(size_t)+block.as_size_t;
	}
	return;
}
void debug(void){
	int rowc = 0;
	for(int i = 0; i < getpagesize(); i++){
		if(rowc==32){
			printf("\n");
			rowc=0;	
		}
		printf("%u ",*(page+i));
		rowc++;
	}
	printf("\n%x, total size %u\n", page, getpagesize());
	return;
}
void malloc_clean(void){
	if(page!=NULL)
		munmap(page, getpagesize());
	return;
}