#include <stdlib.h>
#include <stdio.h>
#include "external.h"

int cbr(int *ptr)
{
	return *ptr;
}

int main(){

	printf("\n\n***********************************************\nTesting call by reference to function in module\n***********************************************\n");
	int n = 1337;
	int *ptr = &n;
	if(cbr(&n) != n)
	{
		printf("\n!!!!!!!UNEXPECTED VALUE RETURNED: %d!!!!!\nEXPECTED:%d\n",cbr(&n),n);
		exit(0);
	}
	if(cbr(ptr) != n)
	{
		printf("\n!!!!!!!UNEXPECTED VALUE RETURNED: %d!!!!!\nEXPECTED:%d\n",cbr(ptr),n);
		exit(0);
	}
	printf("PASS\n");

	printf("\n\n****************************************************\nTesting call by reference to function outside module\n****************************************************\n");
	if(cbr_ext(&n) != n)
	{
		printf("\n!!!!!!!UNEXPECTED VALUE RETURNED: %d!!!!!\nEXPECTED:%d\n",cbr_ext(&n),n);
		exit(0);
	}
	if(cbr_ext(ptr) != n)
	{
		printf("\n!!!!!!!UNEXPECTED VALUE RETURNED: %d!!!!!\nEXPECTED:%d\n",cbr_ext(ptr),n);
		exit(0);
	}
	printf("PASS\n");
}
