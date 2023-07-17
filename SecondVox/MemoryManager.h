#pragma once

#include "Macros.h"

#include "UtilsMath.h"

namespace vox::mem
{


void MMInit();
void MMClear();

// it gives (1 << log2_size) memory
void* MMMemAllocPow2(size_t log2_size);
void* MMMemReallocPow2(void* ptr, size_t log2_size);
void MMMemFree(void* ptr);

// size 0 is not valid
M_FORCE_INLINE void* MMMemAlloc(size_t size)
{
    return MMMemAllocPow2(utils::UMUpperBoundLog2(size));
}
M_FORCE_INLINE void* MMMemRealloc(void* ptr, size_t size)
{
    return MMMemReallocPow2(ptr, utils::UMUpperBoundLog2(size));
 }

};
