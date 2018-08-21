
#include<stdio.h>

const char glob_const_str[] = "This is a global constant string";
char *glob_string;

int main(){

	printf("\n\n************\nTesting global const strings\n************\n");
	const char *local_strptr = &(glob_const_str[10]);
	printf("%s\n",local_strptr);
	printf("PASS\n");

	printf("\n\n************\nTesting global pointers to strings\n************\n");
	glob_string = "Checking global string..\n";
	printf("%s",glob_string);
	printf("PASS\n");
}
