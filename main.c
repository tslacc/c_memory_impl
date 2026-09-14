#include <stdio.h>
#include <unistd.h>
#include "malloc.h"

int main(int argc, char * argv[])
{
	extern uint8_t *page;
	printf("Page ptr is %x\n", page);
	malloc_init();
	printf("Page ptr is %x\n", page);
	debug();
	printf("Getpagesize %u\n", getpagesize());
	for(int i = 0; i<getpagesize(); i++){
		*(page+i)=i%256;
	}
	debug();
	malloc_clean();
	return 0;
}