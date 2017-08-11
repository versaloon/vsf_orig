#include "compiler.h"

unsigned char * compiler_get_heap(void)
{
	#pragma segment="HEAP"
	return __sfb("HEAP");
}

unsigned char * compiler_get_stack(void)
{
	#pragma segment="STACK"
	return __sfe("STACK");
}

