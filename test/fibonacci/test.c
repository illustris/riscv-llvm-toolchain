#include <stdlib.h>
#include <stdio.h>
#include "external.h"

//void printptr(void *a);

unsigned int fib(unsigned int n);

unsigned int fib(unsigned int n){
	if(n < 2)
		return n;
	return fib(n-1) + fib(n-2);
}

int main()
{
	int n;
	printf("Enter a number:\n");
	scanf("%d",&n);
	printf("fib(%u) = %u\n", n, fib(n));
	return 0;
}