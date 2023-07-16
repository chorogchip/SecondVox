#include "MemoryManager.h"

#include <bit>
#include <cstdlib>
#include <malloc.h>

namespace vox::mem
{

static constexpr bool TO_USE_CUSTOM_MEMALLOC = false;

void MMInit()
{
    if constexpr (!TO_USE_CUSTOM_MEMALLOC)
        return;
}
void MMClear()
{
    if constexpr (!TO_USE_CUSTOM_MEMALLOC)
        return;
}

void* MemAllocPow2(size_t log2_size)
{
    if constexpr (!TO_USE_CUSTOM_MEMALLOC)
        return malloc((size_t)1 << log2_size);
}
void* MemReallocPow2(void* ptr, size_t log2_size)
{
    if constexpr (!TO_USE_CUSTOM_MEMALLOC)
    {
        const size_t sz = (size_t)1 << log2_size;
        void* new_mem = malloc(sz);
        memcpy(new_mem, ptr, sz >> 1);
        free(ptr);
        int a = 1;
        return new_mem;
    }
}
void MemFree(void* ptr)
{
    if constexpr (!TO_USE_CUSTOM_MEMALLOC)
        free(ptr);
}

}