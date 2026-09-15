#include <stdio.h>
#include <unistd.h>
#include "malloc.h"

int main(int argc, char * argv[])
{
	extern uint8_t *page;
	printf("Page ptr is %x\n", page);
	malloc_init();
	debug();
	print_blocks();
	malloc_clean();
	return 0;
}