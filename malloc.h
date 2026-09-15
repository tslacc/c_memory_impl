#ifndef LIB_MALLOC
#define LIB_MALLOC
#include <stdint.h>

//Custom malloc implementation.
extern uint8_t *page;
void malloc_init(void);
void debug(void);
void malloc_clean(void);
void print_blocks(void);
#endif