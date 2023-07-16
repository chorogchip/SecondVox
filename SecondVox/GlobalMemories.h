#pragma once

#include "DynamicArray.h"

namespace vox::mem
{

enum class EnumGLobalMemories
{
    BLOCK_LIST_GEN,

    SIZE,

};

void GMInit();
void GMClear();
data::DynamicArray<void *> &GMGetData(EnumGLobalMemories index);
#ifdef M_DEBUG
void GMFreeData(EnumGLobalMemories index);
#else
M_FORCE_INLINE void GMFreeData(EnumGLobalMemories index) {}
#endif

}
