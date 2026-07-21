#include "malloc.h"



typedef struct block{
	char *data;
	char in_use;
} s_block;

s_block *mem_blocks;

//On calling malloc, determine the size of the block necessary.
//Refer to https://linux.die.net/man/3/malloc

void *_malloc(size_t size){
	printf("malloc called with size %u\n",size);
	printf("sbrk(0) returns with value %u\n", sbrk(0));
	//If size is smaller than mmap size, sbrk. If larger, then mmap strategy
	//Mutex possibly necessary
	return sbrk(size);
}