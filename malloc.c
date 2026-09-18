#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "malloc.h"

pthread_mutex_t mutex;
// Store how much space in the immediate next block is used
union blocksize{
	size_t as_size_t;
	char as_char[sizeof(size_t)];
};
// When a block is NOT in use, let the default value be zero; when a block is in use, let its first byte be nonzero
uint8_t *page;
static void write_tag(size_t index, int is_used, size_t size){
	pthread_mutex_lock(&mutex);
	*(page+index) = (uint8_t)is_used;
	union blocksize block;
	block.as_size_t = size;
	memcpy(page+index+1, block.as_char, sizeof(size_t));
	pthread_mutex_unlock(&mutex);
	printf("Wrote tag at (pos, used, size) = (%lu, %u, %lu): ", index, is_used, block.as_size_t);
	for(int i = 0; i < (1+sizeof(size_t)); i++){
		printf("%u ",*(page+index+i));
	}
	printf("\n");
	return;
}
static void custom_init(void){
	pthread_mutex_init(&mutex, NULL);
	if(page==NULL){
		pthread_mutex_lock(&mutex);
		
		page = mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
		
		pthread_mutex_unlock(&mutex);
	}
	write_tag(0, 0, getpagesize()-1-sizeof(size_t));
	return;
}
//Print all blocks within page
void print_blocks(void){
	size_t index = 0;
	printf("sizeof size_t is %lu\n", sizeof(size_t));
	while(index < getpagesize()){
		printf("Scanning index %lu: ", index);
		if(*(page+index)) printf("Block in use");
		else printf("Block is free");
		index++;
		union blocksize block;
		memcpy(block.as_char, page+index, sizeof(size_t));
		printf(", size of block is %lu bits\n", block.as_size_t);
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
	printf("\n%p, total size %u\n", page, getpagesize());
	return;
}
void malloc_clean(void){
	if(page!=NULL)
		munmap(page, getpagesize());
	return;
}

static size_t find_first_open_block_minsize(size_t requested_size){
	if(page == NULL) return -1;
	size_t index = 0;
	union blocksize block;
	// sanity check 
	while(index + 1 + sizeof(size_t) + requested_size < getpagesize()){
		printf("Checking block index %lu\n", index);
		printf("Is this block used? %hhu\n", *(page+index));
		//Get the size of the block
		memcpy(block.as_char, page+index+1, sizeof(size_t));
	
		//is this block free?
		if(*(page+index)==0){
			if(block.as_size_t >= requested_size)
				return index;
		}
		index += 1 + sizeof(size_t) + block.as_size_t;
	}
	return -1;
}
// Returns true if there is space for a valid meaningful block AFTER allocating a block of ideal_size.
// If there isn't, return false.
static int valid_space_after_alloc(size_t index, size_t ideal_size){
	//assume this block is a free block
	union blocksize block;
	memcpy(block.as_char, page+index+1, sizeof(size_t));
	// Is there space to fit a meaningful block?
	const size_t MEANINGFUL_SIZE_MINIMUM = 0;
	if(block.as_size_t-(1+sizeof(size_t))*2 - ideal_size > MEANINGFUL_SIZE_MINIMUM){
		return true;
	}
	return false;
}

// TODO IMPLEMENT
void custom_free(void *ptr){
	return;
}
void *custom_malloc(size_t requestedSize){
	if(page==NULL) custom_init();
	void *result = NULL;
	size_t index = find_first_open_block_minsize(requestedSize);
	if(index != -1) {
		// Is there enough size remaining to fit a small block?
		union blocksize originalSize;
		memcpy(originalSize.as_char, page+index+1, sizeof(size_t));
		if(valid_space_after_alloc(index, requestedSize)){
			write_tag(index, 1, requestedSize);
			write_tag(index+1+sizeof(size_t)+requestedSize, 0, originalSize.as_size_t-(1+sizeof(size_t))*2-requestedSize);
		} else {
			write_tag(index, 1, originalSize.as_size_t-(1+sizeof(size_t)));
		}
		result = page+index+1+sizeof(size_t);
	}
	printf("custom_malloc returning ptr %p\n", result);
	return result;
}
