#include "common_swapper.h"

int swapper_swap_bytes(int n){
	unsigned int i = 1;
	char *c = (char*)&i;

	if(!*c)				//es big
		n = bswap_32(n);

	return n;
}