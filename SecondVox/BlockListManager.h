#pragma once

#include "GameMap.h"

namespace vox::ren
{

struct alignas(8) BLockListData
{
    uint16_t y;
    uint16_t z;
    uint16_t x;
    uint16_t mat12_sz4;
};
static_assert(sizeof(BLockListData) == 8);
static_assert(alignof(BLockListData) == 8);

void BMInit();
void BMClear();
void BMMarkUpdate(wrd::map_size_t y, wrd::map_size_t z, wrd::map_size_t x);
void BMUpdate();

}