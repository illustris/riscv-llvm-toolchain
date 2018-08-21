#include<stdio.h>
int main(){
	
	/*register long int t3 asm ("t3");
	asm volatile ("csrrs t3, 0xC00, x0" "\n\t"
                       :
                       :
                       :"x0", "t3", "cc", "memory");
	*/
	unsigned long long t3 = __builtin_readcyclecounter();
	printf("\n**********************\nTesting array on stack\n**********************\n");
	int x[10];
	for(int i=0;i<10;i++)
	{
		x[i] = 100+i;
	}
	for(int i=0;i<10;i++)
	{
		printf("x[%d] is: %d\n",i,x[i]);
	}
	printf("PASS\n");

	/*register long int t2 asm ("t2");
	asm volatile ("csrrs t2, 0xC00, x0" "\n\t"
                       :
                       :
                       :"x0", "t2", "cc", "memory");
	*/
	unsigned long long t2 = __builtin_readcyclecounter();
	unsigned long long cycles = t3 - t2;
	printf("start : %llu\nend :  %llu\ntotal :  %llu \n",t3,t2,cycles);
}
