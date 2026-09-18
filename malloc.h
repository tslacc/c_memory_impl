#ifndef LIB_MALLOC
#define LIB_MALLOC
#include <stdint.h>

//Custom malloc implementation.
extern uint8_t *page;
void debug(void);
void malloc_clean(void);
void print_blocks(void);
void custom_free(void *ptr);
void *custom_malloc(size_t size);
#endif
