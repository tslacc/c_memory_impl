#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>
#include "malloc.h"

#define SIZEOF_TAG 1+sizeof(size_t)

/* 	TODO
 *	Store multiple pages instead of page
 */
pthread_mutex_t mutex;
union blocksize{
	size_t as_size_t;
	char as_char[sizeof(size_t)];
};
struct page{
	// Base pointer
	uint8_t *bptr;
	size_t freeSpace;
	struct page *nextPage;
};

struct page *basePage;

static struct page *newPage(){
	struct page *result = sbrk(sizeof(struct page));
	result->bptr = NULL;
	result->freeSpace = 0;
	result->nextPage = NULL;
	return result;
}

static void initPages(){
	if(basePage == NULL){
		basePage = newPage();
	}
	return;
}

#undef SIZEOF_TAG
