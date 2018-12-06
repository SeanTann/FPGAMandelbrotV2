/*
 * mathFunctions.c
 *
 *  Created on: 11 Nov 2018
 *      Author: Sean
 */


#include "mathFunctions.h"



long long * add128(unsigned long long *a, unsigned long long *b, unsigned long long *out)
{
	//static long long out[2];

	*out = (*a & 0x7FFFFFFFFFFFFFFF) + (*b & 0x7FFFFFFFFFFFFFFF);
	unsigned int overflow = ((unsigned long long)(*a)>>63) + ((unsigned long long)(*b)>>63) + ((unsigned long long)(*out)>>63);
	*out = (*out & 0x7FFFFFFFFFFFFFFF) | (((unsigned long long)overflow & 0x1)<<63);

	*(out+1) = *(a+1) + *(b+1) + ((overflow>>1) & 0x0000000000000001);

	return out;
}


long long * shiftLeft128(long long *a, int b, long long *out)
{
	//static signed long long out[2];
	unsigned long long *au = (unsigned long long*) a;
	if(b>=0){
		*out = (*au)<<b;
		*(out+1) = ((*(au+1))<<b) | (b>64? (*au)<<(b-64) : (*au)>>(64-b));
	}
	else{
		*(out+1) = (signed long long)(*(a+1))>>(-b);
		*out =(*(au)>>(-b)) | (b<-64 ? ((signed long long)(*(a+1)))>>(-b-64) : (*(au+1))<<(64+b));
	}

	return out;
}


unsigned int * add64(unsigned int *a, unsigned int *b, unsigned int *out)
{
	//static int out[2];

	*out = (*a & 0x7FFFFFFF) + (*b & 0x7FFFFFFF);
	unsigned int overflow = ((unsigned int)(*a)>>31) + ((unsigned int)(*b)>>31) + ((unsigned int)(*out)>>31);
	*out = (*out & 0x7FFFFFFF) | (((unsigned int)overflow & 0x1)<<31);

	*(out+1) = *(a+1) + *(b+1) + ((overflow>>1) & 0x00000001);

	return out;
}


 int * shiftLeft64( int *a, int b,  int *out)
{
	//static signed int out[2];
	unsigned int *au = (unsigned int*) a;
	if(b>=0){
		*out = (*au)<<b;
		*(out+1) = ((*(au+1))<<b) | (b>32? (*au)<<(b-32) : (*au)>>(32-b));
	}
	else{
		*(out+1) = (signed int)(*(a+1))>>(-b);
		*out =(*(au)>>(-b)) | (b<-32 ? ((signed int)(*(a+1)))>>(-b-32) : (*(au+1))<<(32+b));
	}

	return out;
}
