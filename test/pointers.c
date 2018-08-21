#include<stdio.h>
#include<stdlib.h>
int f1()
{
	return 8;
}
int f2()
{
	return 16;
}

int main(){
	register long int t3 asm ("t3");
	asm volatile ("csrrs t3, 0xC00, x0" "\n\t"
                       :
                       :
                       :"x0", "t3", "cc", "memory");


	printf("\n\n************\nTesting null pointer\n************\n");
	int *np;
	np = NULL;
	printf("PASS\n");

	printf("\n\n**************************\nTesting pointer to pointer\n**************************\n");
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

	for(int i=0;i<10;i++)
	{
		free(p[0][i]);
	}
	printf("PASS\n");	

	printf("\n\n************\nTesting function pointer\n************\n");
	typedef int (*functionPtr)();
	functionPtr arrayFp[4];
	arrayFp[0] = f1;
	arrayFp[1] = f2;
	printf("%d, %d\n",arrayFp[0](),arrayFp[1]());
	printf("PASS\n");
	register long int t2 asm ("t2");
	asm volatile ("csrrs t2, 0xC00, x0" "\n\t"
                       :
                       :
                       :"x0", "t2", "cc", "memory");

}
