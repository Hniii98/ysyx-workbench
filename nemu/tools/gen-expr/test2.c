#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main()
{
	int ret = system("gcc test.c -o test");
	//int ret = system("gcc -Wall -Werror test.c -o test");
	printf("ret number is:%d\n", ret);
	return 0;
}


