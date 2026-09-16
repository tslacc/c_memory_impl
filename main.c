#include <stdio.h>
#include <unistd.h>
#include "malloc.h"

int main(int argc, char * argv[])
{
	fflush(stdout);
	printf("pls don't segfault\n");
	return 0;
}