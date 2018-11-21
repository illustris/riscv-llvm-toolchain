#include <stdlib.h>
#include <stdio.h>

#define IV 0xdeadbeef1ee7c0d3;

__int128 craft(unsigned int ptr, unsigned int base, unsigned int bound, unsigned int id);

unsigned long long PRNG = IV;

unsigned long long random64()
{
		PRNG = PRNG ^ (PRNG >> 1) + (PRNG >> 17);
		return PRNG;
}

unsigned int hash_fn(unsigned long long i64)
{
	unsigned int ret = (unsigned int)i64;
	ret = ret ^ (unsigned int)(i64 >> 32);
	//printf("Hash function called\ncook = %llx\nhash = %x\n",i64,ret);
	return ret;
}

/*unsigned int hash_fn_debug(unsigned long long i64)
{
	unsigned int ret = (unsigned int)i64;
	ret = ret ^ (unsigned int)(i64 >> 32);
	//printf("Hash function called\ncook = %llx\nhash = %x\n",i64,ret);
	return ret;
}*/

unsigned long long hash(unsigned long long *cook_ptr)
{
	//printf("\n--------\nHASH 0x%llx\n--------\n",(unsigned long long)cook_ptr);
	*cook_ptr = random64();
	//printf("Setting cookie %llx at address %llx\nhash = %x\n",*cook_ptr,(unsigned long long int)cook_ptr,hash_fn_debug(*cook_ptr));
	return hash_fn(*cook_ptr);
	//return 0;
}
void printptr(__int128 fpr){

	printf("print ptr BOUND:BASE  %016llx\n", (unsigned long long int) (fpr >> 64));
	printf("print ptr IDHASH:PTR  %016llx\n", (unsigned long long int) fpr);
	return;

}

void val(unsigned long long hi, unsigned long long lo)
{
	//printf("\n--------\nVAL 0x%016llx%llx\n--------\n",hi,lo);
	//printf("BOUND:BASE  %016llx\n", (unsigned long long int) (hi));
	//printf("IDHASH:PTR  %016llx\n", (unsigned long long int) lo);
	unsigned int hash = ((lo & 0xffffffff00000000) >> 32);
	unsigned long long *base = (unsigned long long*) ((hi) & 0x00000000ffffffff);
	//void *ptr = (void *)((unsigned long long)lo & 0x7fffffff);// fpr;
	unsigned long long *ptr = (unsigned long long *)(lo & 0x7fffffff);// fpr;
	unsigned long long *bound = (unsigned long long *)((hi & 0xffffffff00000000) >> 32);

	//printf(" base : %llx\n bound : %llx\n pointer : %llx\n hash: %x\n",(unsigned long long)base,(unsigned long long)bound,(unsigned long long)ptr,hash);
	if(base == NULL)
	{
		printf("!!!!VALIDATE ERROR\ngot NULL pointer for base");
		//printf("BOUND:BASE  %016llx\n", (unsigned long long int) (hi));
		//printf("IDHASH:PTR  %016llx\n", (unsigned long long int) lo);
		//exit(0);
		return ;
	}
	if(hash != hash_fn(*base))
	{
		
		printf("!!!!VALIDATE ERROR\ngot hash %x from %08llx\nexpected %x",hash,(unsigned long long)base,hash_fn(*base));
		printf("Validate error for hash\n");
		//exit(0);
		return ;
	}
	if((unsigned long long *)ptr < base || ptr >= bound){
		printf("Validate Error !! pointer access out of range \n");
		//exit(0);
		return;
	}
	return;
}

__int128 safemalloc(unsigned long s)
{
	s+=8;
	register void *ptr = malloc(s);
	unsigned long long cook_hash = hash(ptr);
	__int128 ret = craft((unsigned int)(ptr+8), (unsigned int)ptr, (unsigned int)(ptr+s), cook_hash);
//	ptr = (unsigned int)cook_hash;
	//printf("Malloc got size %lu, pointer %llx\n",s-8,(unsigned long long)ptr);
	return ret;
}

void safefree(__int128 fpr)
{
	//printf("free BOUND:BASE  %016llx\n", (unsigned long long int) (fpr >> 64));
	//printf("free IDHASH:PTR  %016llx\n", (unsigned long long int) fpr);
	void* ptr_high = (void*)(fpr>>64);
	val((unsigned long long)(ptr_high),(unsigned long long)fpr);
	//ptr_high = (void*)((unsigned long long)ptr & 0x7fffffff);
	void *ptr = (void *)((unsigned long long)fpr & 0x7fffffff);// fpr;
	//printf("FREE called with %llx\n",(unsigned long long)ptr);
	ptr = ptr -8;
	//printf("Moving to base %llx",(unsigned long long)ptr);
	free(ptr);
	return;
}

/*void printptr(void *a)
{
	printf("Pointer: %llx",(unsigned long long)a);
	return;
}*/
