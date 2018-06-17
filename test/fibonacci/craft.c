#include <stdio.h>
__int128 craft(unsigned int ptr, unsigned int base, unsigned int bound, unsigned int id)
{
	//printf("Called craft ptr = %x, base = %x\n",ptr, base);
	__int128 ret;
	ret = ptr;
	ret ^= ((__int128)id << 32);
	ret ^= ((__int128)base << 64);
	ret ^= ((__int128)bound << 96);
	return ret;
}