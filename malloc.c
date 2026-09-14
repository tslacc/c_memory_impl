#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "malloc.h"
uint8_t *page;
void malloc_init(void){
	if(page==NULL)
		page = mmap(NULL, getpagesize(), PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
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
	printf("\n%x, total size %u\n", page, getpagesize());
	return;
}
void malloc_clean(void){
	if(page!=NULL)
		munmap(page, getpagesize());
	return;
}