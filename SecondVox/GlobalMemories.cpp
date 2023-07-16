#include "GlobalMemories.h"

#include "Macros.h"

namespace vox::mem
{

static data::DynamicArray<void *> datas_[(size_t)EnumGLobalMemories::SIZE];
#ifdef M_DEBUG
bool is_data_took_[(size_t)EnumGLobalMemories::SIZE];
#endif


void GMInit()
{
    for (int i = 0; i < (int)EnumGLobalMemories::SIZE; ++i)
    {
        datas_[i].Init(8);
    }
}
void GMClear()
{
    for (int i = 0; i < (int)EnumGLobalMemories::SIZE; ++i)
    {
        datas_[i].Clear();
    }
}
data::DynamicArray<void *> &GMGetData(EnumGLobalMemories index)
{
#ifdef M_DEBUG
    assert(is_data_took_[(size_t)index] == false);
    is_data_took_[(size_t)index] = true;
#endif
    return datas_[(size_t)index];
}

#ifdef M_DEBUG
void GMFreeData(EnumGLobalMemories index)
{
    assert(is_data_took_[(size_t)index] == true);
    is_data_took_[(size_t)index] = false;
}
#endif

}