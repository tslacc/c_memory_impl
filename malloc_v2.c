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
	void *bptr;
	struct Page *nextPage;
};

struct Page *basePage;

// Write a u32 tag
static void writeTag(uint8_t *memPtr, bool inUse, uint32_t sz){
	union U32 tag;
	tag.as_u32 = ((sz << 1) >> 2) | (inUse << 31);
	memcpy(memPtr, tag.as_char, SIZEOF_TAG);
}
//Returns the highest bit
static uint8_t getBlockUsed(uint32_t tag){
	return (uint8_t)(tag>>31);
}
static uint32_t getBlockSize(uint32_t tag){
	return (tag<<1)>>1;
}
static uint32_t getBlockTag(void *memPtr){
	uint32_t result = 0;
	memcpy(&result, memPtr, SIZEOF_TAG);
	return result;
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
// Accepts two params: bptr, the base address for the page, and *cptr, the current tag's address
static void *get_ptr_to_next_tag(struct Page *page, void *cptr){
	uint32_t dist = getBlockTag(cptr);
	dist = (dist << 1) >> 1;
	if(cptr+dist - (void*)page->bptr > (uint32_t)PAGE_SIZE)
		return NULL;
	return cptr+dist+SIZEOF_TAG;
}
static void *search_supremum_block(const uint32_t targetSize){
	printf("Supremum search for size %lu\n", targetSize);
	struct Page *page = basePage;
	void *result = NULL;
	uint32_t record_size = UINT32_MAX;
	while(page != NULL){
		void *pos = page->bptr;
		printf("Supremum search with base pointer %p\n", page->bptr);
		while(pos!=NULL){
			uint32_t size = getBlockSize(getBlockTag(pos));
			if(size >= targetSize && size < record_size){
				result = pos;
				record_size = size;
			}
			pos = get_ptr_to_next_tag(page, pos);
		}
		page = page->nextPage;
	}
	return result;
}
static struct Page *identify_page_of_ptr(const void *ptr){
	struct Page *result = basePage;
	while(result!=NULL){
		if((void*)(result->bptr)+PAGE_SIZE > ptr) return result;
		result = result->nextPage;
	}
	return NULL;
}
static void *getPtrLeftTag(struct Page *page, const void *ptr){
	void *result = page->bptr;
	while(true){
		void *next = get_ptr_to_next_tag(page, result);
		if(next > ptr) return result;
		result = next;
	}
	return result;
}
static void merge_free_right(struct Page *page, void *ptr){
	uint32_t tag = getBlockTag(ptr-SIZEOF_TAG);
	uint32_t *p_nextTag = get_ptr_to_next_tag(page, ptr);
	uint32_t nextTag = getBlockTag(p_nextTag);
	if(getBlockUsed(nextTag)) return;
	writeTag(ptr, 0, getBlockSize(tag) + SIZEOF_TAG + getBlockSize(nextTag));
}
static void merge_free_left(struct Page *page, void *ptr){
	void *left = getPtrLeftTag(page, ptr);
	if(getBlockUsed(getBlockTag(left))) return;
	merge_free_right(page, left);	
}
// Accepts a pointer to allocated memory and frees the HEADER.
void custom_free(void *ptr){
	// Decrement to header
	ptr = ptr - SIZEOF_TAG;
	struct Page *page = identify_page_of_ptr(ptr);
	merge_free_right(page, ptr);
	// Only merge if necessary
	merge_free_left(page, ptr);
	return;
}

static void initialize(){
	pthread_mutex_init(&mutex, NULL);
	basePage = newPage();
}

void debug(void){
}
// Aligns to 8
static size_t align8(size_t size){
	uint8_t adjust = (0b1000-(size&0b111))&0b111;
	size = size+adjust;
	return size;
}
// TODO Upgrade: Doesn't track huge pages in a smart way
static void *hugeCustomMalloc(size_t size){
	void *result = sbrk(sizeof(void *));
	result = NULL;
	pthread_mutex_lock(&mutex);
	result = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
	pthread_mutex_unlock(&mutex);
	printf("Huge malloc allocated at %p\n", result);
	return result;
}
void *custom_malloc(size_t size){
	printf("Begin cmalloc w sz %u out of standard size %u\n", size, PAGE_SIZE);
	if(size+SIZEOF_TAG > (size_t)PAGE_SIZE) return hugeCustomMalloc(size);
	size = align8(size);
	if(basePage == NULL) initialize();
	void *result = search_supremum_block(size);
	printf("Supremum result position is %p\n", result);
	if(result==NULL) return NULL;
	writeTag(result, 1, size);
	return result;
}
#undef SIZEOF_TAG
#undef PAGE_SIZE
