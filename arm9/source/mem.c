#include <stdlib.h>
#include "mem.h"

static uint32_t allocated = 0;

void* mem_alloc(uint32_t size)
{
	void* ptr = malloc(size+4);
	*((uint32_t*)ptr) = size+4;
	allocated += size+4;
	return ptr+4;
}

void mem_free(void* ptr)
{
	if (ptr)
	{
		ptr -= 4;
		uint32_t size = *((uint32_t*)ptr);
		allocated -= size;
		free(ptr);
	}
}

uint32_t mem_get_allocated()
{
	return allocated;
}
