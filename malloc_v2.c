#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>
#include "malloc.h"

#define SIZEOF_TAG sizeof(uint32_t)
#define PAGE_SIZE sysconf(_SC_PAGESIZE)
/* 	TODO
 *	Store multiple pages instead of page
 */
pthread_mutex_t mutex;

union U32{
	uint32_t as_u32;
	char as_char[sizeof(size_t)];
};
// How many blocks can fit in a page?
// If a block has 4096 (4kB) pagesize, then potentially we need 512 blocks
// To store a block, we need to store the base pointer and the size of the block itself
// Base pointer can be an offset from bptr, then at most we need to store a number that can hold 4096, i.e. a uint12, so a uint16 is appropriate for this...
// We additionally need to store the size of the bptr, so double it and we have a total storage size of 32 bits, 4byte
// To store 512 blocks we would then need 2048 bytes, or half a page's worth of memory.
// We would then need to traverse all the blocks to find a valid pointer or a blank, then write it: this is O(n) unless the array is presorted, which gives O(nlogn) for sorting speed
//
// Alternatively we can encode the block size and usage directly into the page itself: we can consume 8 total bytes (32bit) for alignment
// Let the first bit of the first byte be 1 to represent a used block, and 0 for unused blocks
// Let the remaining 31bit be a size_t (32bit) with the MSB removed via bit shifting: we are left with 31 bits to fit in the remaining space.
// Then the page itself becomes a linked list, with each block header pointing to the next
// Disadvantage is that the block headers are no longer distinct: we cannot guarantee that free() will be freeing a real pointer
// O(n) search and O(1) allocation time.
struct Page{
	// Base pointer
	uint8_t *bptr;
	struct Page *nextPage;
};

struct Page *basePage;

static void writeTag(uint8_t *memPtr, bool inUse, uint32_t blockSize){
	union U32 tag;
	tag.as_u32 = ((blockSize << 1) >> 2) + (inUse << 31);
	memcpy(memPtr, tag.as_char, sizeof(uint32_t));
}

static struct Page *newPage(){
	struct Page *result = sbrk(sizeof(struct Page));
	result->bptr = NULL;
	pthread_mutex_lock(&mutex);
	result->bptr = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
	writeTag(result->bptr, false, PAGE_SIZE - SIZEOF_TAG);
	pthread_mutex_unlock(&mutex);
	result->nextPage = NULL;
	return result;
}
static void initialize(){
	pthread_mutex_init(&mutex, NULL);
	basePage = newPage();
}
void debug(void){
}
void *custom_malloc(const size_t size){
	if(basePage == NULL) initialize();
	return NULL;
}
#undef SIZEOF_TAG
#undef PAGE_SIZE
