#include <stdlib.h>
#include <stdio.h>
#include "external.h"

//void printptr(void *a);

void test();
int rec_count = 0;
char tc = 'a';
void test(){
	char a[1024][1024];
	a[0][0] = tc;
	tc++;
	//printf("Stack at %llx\n",(unsigned long long)a);
	//printf("%x\t%x\n",*((char*)((unsigned long long)a & 0x00000000ffffffff)),*(char*)a);
	printf("rec count %d:\t%c\n",rec_count,a[0][0]);
	rec_count++;
	//fflush(stdout);
	if(rec_count != 6)
		test();
}

int cbr(int *ptr)
{
	return *ptr;
}

int f1()
{
	return 8;
}
int f2()
{
	return 16;
}

int main()
{

	printf("\n**********************\nTesting array on stack\n**********************\n");
	//int a = 0x1ee7c0de;
	int x[10];
	//x=13;
	//printf("%d\n",x);
	for(int i=0;i<10;i++)
	{
		x[i] = 100+i;
	}
	for(int i=0;i<10;i++)
	{
		printf("x[%d] is: %d\n",i,x[i]);
	}
	printf("PASS\n");

	printf("\n\n*****************\nTesting recursion\n*****************\n");
	test();
	printf("PASS\n");

	printf("\n\n*******************\nTesting malloc-free\n*******************\n");
	char *q = malloc(10);
	free(q);
	printf("PASS\n");

	printf("\n\n**************\nTesting malloc\n**************\n");
	int *p[10][10];
	for(int i=0;i<10;i++)
	{
		p[0][i] = malloc(40);
		for(int j=0;j<10;j++)
			p[0][i][j] = 100*i+j;
		for(int j=0;j<10;j++)
		{
			//printf("%d:\tp[%d] = %d\n",i,j,p[j]);
			if(p[0][i][j] != 100*i+j)
			{
				printf("\n!!!!!!!VALUE STORED IN HEAP CLOBBERED!!!!!\n");
				exit(0);
			}
		}
	}
	printf("PASS\n");

	printf("\n\n**************************\nTesting pointer to pointer\n**************************\n");
	int *r;
	for(int i=0;i<10;i++)
	{
		for(int j=0;j<10;j++)
		{
			r = p[0][i];
			if(r[j] != 100*i+j)
			{
				printf("\n!!!!!!!UNEXPECTED VALUE AT (%d, %d, %d)!!!!!\n",0,i,j);
				exit(0);
			}
		}
	}
	printf("PASS\n");


	printf("\n\n************\nTesting free\n************\n");
	for(int i=0;i<10;i++)
	{
		free(p[0][i]);
	}
	printf("PASS\n");

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

	printf("\n\n************\nTesting function pointer\n************\n");
	typedef int (*functionPtr)();
	functionPtr arrayFp[4];
	arrayFp[0] = f1;
	arrayFp[1] = f2;
	printf("%d, %d\n",arrayFp[0](),arrayFp[1]());
	printf("PASS\n");

	printf("\n\n************\nTesting null pointer\n************\n");
	int *np;
	np = NULL;
	printf("PASS\n");
	return 0;
}
