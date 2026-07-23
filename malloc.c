#include "malloc.h"


/*On calling malloc, determine the size of the block necessary.
//Refer to https://linux.die.net/man/3/malloc
typedef struct block{
	char *data;
	char in_use;
} s_block;
s_block *mem_blocks;
*/

size_t *base_address;

static char mutex = 0;

void *_malloc(size_t size){
	if(base_address == NULL){
		base_address = sbrk(0);
	}
	printf("Base address starts at %u\n", base_address);
	//If size is smaller than mmap size, sbrk. If larger, then mmap strategy
	//Mutex possibly necessary
	//Write the size of the block before the block itself. Limit 4-byte block size
	if(!(size < 4294967296)) return NULL;
	while(mutex!=0); //Basic spinlock
	mutex=1;
	*(unsigned int*)sbrk(4) = size;
	printf("Allocating %u size at position %u, size written to %u\n", size, sbrk(0), sbrk(0)-4);
	mutex=0;
	return sbrk(size);
}