#include <stdlib.h>
#include <stdio.h>

__int128 craft(unsigned int ptr, unsigned int base, unsigned int bound, unsigned int id);

unsigned long long hash(unsigned long long *cook_ptr)
{
        *cook_ptr = 0xdeadbeef1ee7c0d3;
        return 0xbeefdead;
		//return 0;
}

void val(unsigned long long hi, unsigned long long lo)
{
	//printf("\n--------\nVAL 0x%llx%llx\n--------\n",hi,lo);
	return;
}

__int128 safemalloc(unsigned long s)
{
	s+=8;
	register void *ptr = malloc(s);
	unsigned long long cook_hash = hash(ptr);
	__int128 ret = craft((unsigned int)(ptr+8), (unsigned int)ptr, (unsigned int)(ptr+s), cook_hash);
	printf("Malloc got size %lu, pointer %llx\n",s-8,(unsigned long long)ptr);
	return ret;
}

void safefree(__int128 fpr)
{
	//printf("BOUND:BASE  %016llx\n", (unsigned long long int) (fpr >> 64));
	//printf("IDHASH:PTR  %016llx\n", (unsigned long long int) fpr);
	void* ptr = (void*)(fpr>>64);
	val((unsigned long long)(ptr),(unsigned long long)fpr);
	ptr = (void*)((unsigned long long)ptr & 0x7fffffff);
	printf("FREE called with %llx\n",(unsigned long long)ptr);
	//ptr = ptr-8;
	//printf("Moving to base %llx",(unsigned long long)ptr);
	free(ptr);
	return;
}

void printptr(void *a)
{
	printf("Pointer: %llx",(unsigned long long)a);
	return;
}
