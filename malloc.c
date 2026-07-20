#include "malloc.h"

struct block{
	char *data;
	char in_use;
} s_block;

static s_block *mem_blocks;

//On calling malloc, determine the size of the block necessary.
//Split block sizes into at least three groups (small, med, large)
//Preallocate a large contiguous block of memory and divide into blocks
//Determine a free block and give to caller
void *_malloc(size_t size){
	printf("malloc called with size %u\n",size);
	return (void*)10;
}