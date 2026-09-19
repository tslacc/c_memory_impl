#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>
#include "malloc.h"

#define SIZEOF_TAG 1+sizeof(size_t)

pthread_mutex_t mutex;
// Store how much space in the immediate next block is used
union blocksize{
	size_t as_size_t;
	char as_char[sizeof(size_t)];
};
// When a block is NOT in use, let the default value be zero; when a block is in use, let its first byte be nonzero
uint8_t *page;
static void write_tag(uint8_t *index, int is_used, size_t size){
	if(index == NULL) return;
	pthread_mutex_lock(&mutex);
	*index = (uint8_t)is_used;
	union blocksize block;
	block.as_size_t = size;
	memcpy(index+1, block.as_char, sizeof(size_t));
	pthread_mutex_unlock(&mutex);
	printf("Wrote tag at (pos, used, size) = (%p, %u, %lu): ", index, is_used, block.as_size_t);
	for(int i = 0; i < (1+sizeof(size_t)); i++){
		printf("%u ",*(index+i));
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
	printf("Custom init attempting to write tag to %p\n", page);
	write_tag(page, 0, getpagesize()-1-sizeof(size_t));
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

// Find the index of a block corresponding to supremum of requested size
// Return the header index if such a block exists
// Return NULL if it doesn't exist
static uint8_t *find_ptr_of_supremum_block(size_t requested_size){
	uint8_t *result = NULL;
	uint8_t *index = page;
	size_t local_block_size = 0;
	size_t min_record_size = INT_MAX;
	while(index + SIZEOF_TAG + requested_size < page+getpagesize()){
		memcpy(&local_block_size, index+1, sizeof(size_t));
		if(*index==0) //This block is free
			if(requested_size <= local_block_size) // This block is big enough
				if(local_block_size < min_record_size) // This block beats the supremum record
					result = index;
		index += SIZEOF_TAG + local_block_size;
	}
	return result;
}

// Returns true if there is space for a valid meaningful block AFTER allocating a block of ideal_size.
// If there isn't, return false.
static int valid_space_after_alloc(uint8_t *index, size_t ideal_size){
	//assume this block is a free block
	union blocksize block;
	memcpy(block.as_char, index+1, sizeof(size_t));
	// Is there space to fit a meaningful block?
	const size_t MEANINGFUL_SIZE_MINIMUM = 0;
	if(block.as_size_t-(1+sizeof(size_t))*2 - ideal_size > MEANINGFUL_SIZE_MINIMUM){
		return true;
	}
	return false;
}
// Find the last valid block that exists before a threshold
// TODO Implement
static uint8_t *find_ptr_last_block_before_pos(uint8_t *stop_ptr){
	return NULL;
}
// Assumes that *ptr is valid
static void merge_to_right(uint8_t *ptr){
	union blocksize local_block_size;
	memcpy(local_block_size.as_char, ptr+1, sizeof(size_t));
	if(*(ptr+SIZEOF_TAG+local_block_size.as_size_t) == 0){ // Block is free, merge. Do NOT need to write all zeros
		void *merge_position = (ptr+SIZEOF_TAG+local_block_size.as_size_t);
		size_t merge_blocksize = 0;
		memcpy(&merge_blocksize, merge_position+1, sizeof(size_t));
		local_block_size.as_size_t += SIZEOF_TAG + merge_blocksize;
		//Manually write new size into local_block_size position
		memcpy(ptr+1, local_block_size.as_char, sizeof(size_t)); 
	}
}
void custom_free(uint8_t *ptr){
	//TODO Sanity check
	// Set self pointer to 0
	*ptr = 0;
	// Merge to RIGHT
	merge_to_right(ptr);
	// Merge to LEFT
	uint8_t *left_block_ptr = find_ptr_last_block_before_pos(ptr);
	if(left_block_ptr != NULL) { // A left block exists
		if(*left_block_ptr == 0) { //This is a simple free block, merge to right
			merge_to_right(left_block_ptr);
		} // Else this is not a free block and we can ignore it
	}
	return;
}
void *custom_malloc(size_t requestedSize){
	printf("Begin custom malloc w rs %lu\n", requestedSize);
	if(page==NULL) custom_init();
	printf("custom init OK\n");
	void *result = NULL;
	uint8_t *index = find_ptr_of_supremum_block(requestedSize);
	if(index != NULL) {
		// Is there enough size remaining to fit a small block?
		union blocksize originalSize;
		memcpy(originalSize.as_char, index+1, sizeof(size_t));
		if(valid_space_after_alloc(index, requestedSize)){
			write_tag(index, 1, requestedSize);
			write_tag(index+1+sizeof(size_t)+requestedSize, 0, originalSize.as_size_t-(1+sizeof(size_t))*2-requestedSize);
		} else {
			write_tag(index, 1, originalSize.as_size_t-(1+sizeof(size_t)));
		}
		result = index+SIZEOF_TAG;
	}
	printf("custom_malloc returning ptr %p\n", result);
	return result;
}
