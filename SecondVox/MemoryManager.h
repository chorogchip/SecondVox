#pragma once

#include "Macros.h"

#include "UtilsMath.h"

namespace vox::mem
{


void MMInit();
void MMClear();

// it gives (1 << log2_size) memory
void* MemAllocPow2(size_t log2_size);
void* MemReallocPow2(void* ptr, size_t log2_size);
void MemFree(void* ptr);

// size 0 is not valid
M_FORCE_INLINE void* MemAlloc(size_t size)
{
    return MemAllocPow2(utils::UpperBoundLog2(size));
}
M_FORCE_INLINE void* MemRealloc(void* ptr, size_t size)
{
    return MemReallocPow2(ptr, utils::UpperBoundLog2(size));
 }

};
