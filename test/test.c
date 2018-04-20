#include <stdlib.h>
#include <stdio.h>

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

int main()
{
	//printf("\n\n**************\nTesting N-D malloc-free\n**************\n");
	int *p[10][10];
	//p[0][0] = malloc(40);
	//free(p[0][0]);

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

	printf("\n\n*****************\nTesting recursion\n*****************\n");
	test();

	printf("\n\n**************\nTesting malloc-free\n**************\n");
	char *q = malloc(10);
	free(q);
	printf("PASS\n");

	printf("\n\n**************\nTesting malloc\n**************\n");
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
				printf("\n!!!!!!!VALUE STORED IN HEAP CLOBBERED!!!!!");
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

	return 0;
}
