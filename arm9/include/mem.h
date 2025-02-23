#ifndef MEM_H_INCLUDED
#define MEM_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void* mem_alloc(uint32_t size);
void mem_free(void* ptr);
uint32_t mem_get_allocated();

#ifdef __cplusplus
}
#endif

#endif // MEM_H_INCLUDED
