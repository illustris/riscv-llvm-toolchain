
#include<stdio.h>
#include<stdlib.h>

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

int main(){

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
}
