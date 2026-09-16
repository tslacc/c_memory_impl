#ifndef LIB_MALLOC
#define LIB_MALLOC
#include <stdint.h>

//Custom malloc implementation.
extern uint8_t *page;
void custom_init(void);
void debug(void);
void malloc_clean(void);
void print_blocks(void);
void *custom_malloc(size_t size);
#endif