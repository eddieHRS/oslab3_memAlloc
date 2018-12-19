#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "mem.h"

int m_error = 0;
int main()
{
	//printf("%d\n",getpagesize());
	mem_init(4096);
	void* a = mem_alloc(5000,M_FIRSTFIT);
	void* b = mem_alloc(96,M_FIRSTFIT);
	void* c = mem_alloc(200,M_FIRSTFIT);
	void* d = mem_alloc(96,M_BESTFIT);
	mem_free(c);
	mem_free(a);
	mem_dump();
	a = mem_alloc(20,M_BESTFIT);
	mem_dump();
	mem_free(a);
	mem_dump();
	printf("%s\n","************" );
	mem_free(b);
	mem_dump();
	mem_free(d);
	mem_dump();



}
