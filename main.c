#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "malloc.h"

int main(int argc, char * argv[])
{
	char *test_string = custom_malloc(5);
	memcpy(test_string, "TEST\0", 5);
	printf("%p, %s\n", test_string, test_string);
	char *test_string2 = custom_malloc(6);
	memcpy(test_string2, "TEST2\0", 6);
	printf("%p, %.8s\n", test_string2, test_string2);
	debug();
	return 0;
}
