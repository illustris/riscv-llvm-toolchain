#include<stdio.h>

int rec_count = 0;
char tc = 'a' ;

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

int main(){

	printf("\n\n*****************\nTesting recursion\n*****************\n");
	rec();
	printf("PASS\n");
}
