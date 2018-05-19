#include <stdlib.h>
#include <stdio.h>
#include "external.h"

//void printptr(void *a);

void rec();
int rec_count = 0;
char tc = 'a';
void rec(){
	char a[1024][1024];
	a[0][0] = tc;
	tc++;
	//printf("Stack at %llx\n",(unsigned long long)a);
	//printf("%x\t%x\n",*((char*)((unsigned long long)a & 0x00000000ffffffff)),*(char*)a);
	printf("rec count %d:\t%c\n",rec_count,a[0][0]);
	rec_count++;
	//fflush(stdout);
	if(rec_count != 6)
		rec();
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

struct simple
{
	int a;
	char b[10];
};

struct nested
{
	int a;
	char b[10];
	struct simple s1;
};

struct with_pointer
{
	int a;
	char b[10];
	struct simple *s1;
};

int *gptr;
struct with_pointer *gptr_tostruct;

const char glob_const_str[] = "This is a lobal constant string";

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
	rec();
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

	printf("\n\n************\nTesting structs\n************\n");
	struct simple s1;
	s1.a = 10;
	s1.b[3] = 'x';
	if(s1.a == 10 && s1.b[3] == 'x')
		printf("s1.a = %d, s1.b[3] = %c\n",s1.a,s1.b[3]);
	else
	{
		printf("!!!!!!!UNEXPECTED VALUE!!!!!!!\ns1.a = %d, s1.b[3] = %c\n",s1.a,s1.b[3]);
		exit(0);
	}
	struct nested n1;
	n1.s1.a = 10;
	n1.s1.b[3] = 'x';
	if(n1.s1.a == 10 && n1.s1.b[3] == 'x')
		printf("n1.s1.a = %d, n1.s1.b[3] = %c\n",n1.s1.a,n1.s1.b[3]);
	else
	{
		printf("!!!!!!!UNEXPECTED VALUE!!!!!!!\ns1.a = %d, s1.b[3] = %c\n",n1.s1.a,n1.s1.b[3]);
		exit(0);
	}

	struct with_pointer swp1;
	swp1.s1 = &s1;
	swp1.s1->a = 20;
	swp1.s1->b[3] = 'z';
	if(swp1.s1->a == 20 && swp1.s1->b[3] == 'z')
		printf("swp1.s1->a = %d, swp1.s1->b[3] = %c\n",swp1.s1->a,swp1.s1->b[3]);
	else
	{
		printf("!!!!!!!UNEXPECTED VALUE!!!!!!!\ns1.a = %d, s1.b[3] = %c\n",swp1.s1->a,swp1.s1->b[3]);
		exit(0);
	}

	swp1.s1 = malloc(sizeof(struct simple));
	swp1.s1->a = 20;
	swp1.s1->b[3] = 'z';
	if(swp1.s1->a == 20 && swp1.s1->b[3] == 'z')
		printf("swp1.s1->a = %d, swp1.s1->b[3] = %c\n",swp1.s1->a,swp1.s1->b[3]);
	else
	{
		printf("!!!!!!!UNEXPECTED VALUE!!!!!!!\ns1.a = %d, s1.b[3] = %c\n",swp1.s1->a,swp1.s1->b[3]);
		exit(0);
	}
	free(swp1.s1);

	struct with_pointer *swpptr = malloc(sizeof(struct with_pointer));
	swpptr->a = 10;
	swpptr->b[3] = 'z';
	swpptr->s1 = malloc(sizeof(struct simple));
	swpptr->s1->a = 30;
	swpptr->s1->b[5] = 'k';
	if(swpptr->a == 10 && swpptr->b[3] == 'z' && swpptr->s1->a == 30 && swpptr->s1->b[5] == 'k')
	{
		printf("swpptr->a == %d && swpptr->b[3] == %c && swpptr->s1->a == %d && swpptr->s1->b[5] = %c\n",swpptr->a,swpptr->b[3],swpptr->s1->a,swpptr->s1->b[5]);
	}
	else
	{
		printf("!!!!!!!UNEXPECTED VALUE!!!!!!!\n");
	}

	printf("PASS\n");

	printf("\n\n************\nTesting global pointer\n************\n");
	int loc[3];// = {1337,1338,1339};
	int *locp;
	loc[1]=1337;
	gptr = loc;
	locp = loc;
	printf("%d\t%d\n",gptr[1],locp[1]);

	gptr_tostruct = malloc(sizeof(struct with_pointer));
	gptr_tostruct->a = 10;
	gptr_tostruct->b[3] = 'z';
	gptr_tostruct->s1 = malloc(sizeof(struct simple));
	gptr_tostruct->s1->a = 30;
	gptr_tostruct->s1->b[5] = 'k';
	if(gptr_tostruct->a == 10 && gptr_tostruct->b[3] == 'z' && gptr_tostruct->s1->a == 30 && gptr_tostruct->s1->b[5] == 'k')
	{
		printf("gptr_tostruct->a == %d && gptr_tostruct->b[3] == %c && gptr_tostruct->s1->a == %d && gptr_tostruct->s1->b[5] = %c\n",gptr_tostruct->a,gptr_tostruct->b[3],gptr_tostruct->s1->a,gptr_tostruct->s1->b[5]);
	}
	else
	{
		printf("!!!!!!!UNEXPECTED VALUE!!!!!!!\n");
	}
	printf("PASS\n");

	printf("\n\n************\nTesting reentrant function\n************\n");
	fflush(stdout);
	printf("PASS\n");

	printf("\n\n************\nTesting global const strings\n************\n");
	const char *local_strptr = &(glob_const_str[10]);
	printf("%s\n",local_strptr);
	printf("PASS\n");

	return 0;
}
