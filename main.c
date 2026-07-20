#include <stdio.h>
#include "malloc.h"

int main(int argc, char * argv[])
{
	printf("Malloc funct signature is %u\n", _malloc);
	void *foo = _malloc(10);
	printf("Ok, foo is %u\n", foo);
	return 0;
}