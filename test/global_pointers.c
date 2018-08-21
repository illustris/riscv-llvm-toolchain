
#include<stdio.h>
#include<stdlib.h>

struct with_pointer
{
	int a;
	char b[10];
	struct simple *s1;
};

struct simple
{
	int a;
	char b[10];
};

struct with_pointer *gptr_tostruct;
int *gptr;

int main(){

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

}
