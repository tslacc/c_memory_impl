#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <malloc.h>

#define INITIAL_BITS 4
// Only write tags to data instead of the pointer itself
struct ptrHashmap{
	uint32_t **data;
	uint8_t currentBitsInHash;
	uint64_t map_entries;
};

// Compute hash where pointer itself is the key
static uint64_t basicComputeHash(struct ptrHashmap *hashmap, void *ptr){
	uint64_t mask = (1<<hashmap->currentBitsInHash) - 1;
	// Detect collision and expand data AND currentBitsInHash if needed
	return (uint64_t)ptr & mask;
}

struct ptrHashmap *getConstructedHashmap(){
	struct ptrHashmap *result = malloc(sizeof(struct ptrHashmap));
	result->currentBitsInHash = 4;
	// Data holds 2^n -1 pointers
	result->data = mmap(NULL, ((1<<INITIAL_BITS) -1)*(sizeof(void *)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
	return result;
}

void ptrHashmap_addPointerToDict(struct ptrHashmap *hashmap, void *ptr, uint32_t tag){
	// Get the offset hash
	uint64_t offset_hash = basicComputeHash(hashmap, ptr);
	//Check for collision
	while(*(hashmap->data+offset_hash)!=NULL && *(hashmap->data+offset_hash)!=ptr){ //we have a collision
		if(hashmap->currentBitsInHash < 64){ //Expand number of bits to get
			hashmap->currentBitsInHash ++;
			// TODO Somehow expand the data pointer...?
		}
		offset_hash = basicComputeHash(hashmap, ptr);
		
	}
	memcpy((hashmap->data+basicComputeHash(hashmap, ptr)), &tag, sizeof(uint32_t));
}
