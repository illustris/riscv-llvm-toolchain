#include<stdio.h>
#include<stdlib.h>
int main(){

	printf("\n\n*******************\nTestiting malloc-free\n*******************\n");
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

	printf("\n\n************\nTesting free\n************\n");
	for(int i=0;i<10;i++)
	{
		free(p[0][i]);
	}
	printf("PASS\n");

}
