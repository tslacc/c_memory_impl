#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#define INITIAL_BITS 4
// Keep all written tags in here?
struct ptrHashmap{
	void **data;
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
	// Data holds 2^n -1 pointers
	result->data = mmap(NULL, ((1<<INITIAL_BITS) -1)*(sizeof(void *)), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
	return result;
}

void ptrHashmap_addPointerToDict(struct ptrHashmap *hashmap, void *ptr){
	// Store pointer at the offset computed by ComputeHash
	*(hashmap->data+basicComputeHash(hashmap, ptr)) = ptr;
	
	
}
