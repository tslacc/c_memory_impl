#ifndef LIB_MALLOC
#define LIB_MALLOC

#include <stddef.h>
#include <stdio.h>
#define DYNAMIC_ALLOCATOR_BLOCK_SIZE 512
#define DYNAMIC_ALLOCATOR_BLOCK_COUNT 4

//Custom malloc implementation.
void *_malloc(long unsigned int size);

#endif