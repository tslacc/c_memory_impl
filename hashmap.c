#include <stdint.h>
#include <stdlib.h>

// Keep all written tags in here?
struct ptrHashmap{
	void **data;
	uint64_t map_entries;
	uint64_t (*getHashOffset)(void *ptr);
};

// Compute hash where pointer itself is the key
static uint64_t basicComputeHash(void *ptr){
	return (uint64_t)ptr%UINT32_MAX;
}

struct ptrHashmap *getConstructedHashmap(){
	struct ptrHashmap *result = malloc(sizeof(struct ptrHashmap));
	result->getHashOffset = basicComputeHash;
	return result;
}
