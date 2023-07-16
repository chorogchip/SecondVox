#pragma once

#include <cstdint>
#include <numeric>

#include "Macros.h"
#include "UtilsRandom.h"
#include "UtilsSave.h"
#include "DynamicArray.h"

#include "GameBlock.h"

namespace vox::wrd
{

class GameMap;

struct alignas(16) MapNode
{
private:
    using map_node_t = int16_t;
    map_node_t childs_[8];
    static inline data::DynamicArray<int32_t> far_diffs_;

    constexpr static map_node_t FLAG_BITS = 0b1111;
    constexpr static map_node_t DIFF_BITS = (map_node_t)-1 ^ FLAG_BITS;
    constexpr static map_node_t FLAG_SHR_CNT = 4;

    constexpr static map_node_t FLAG_IS_ALL_FULL = 0b1;
    constexpr static map_node_t FLAG_IS_SOMEADJ_FULL = 0b10;
    constexpr static map_node_t FLAG_IS_FAR_PTR = 0b100;
    constexpr static map_node_t FLAG_IS_LEAF = 0b1000;

    constexpr static int64_t MAX_SHORT_DIFF = (int64_t)std::numeric_limits<map_node_t>::max();
    constexpr static game_block_t MAX_SHORT_BLOCK_DIFF = std::numeric_limits<game_block_t>::max() >> FLAG_SHR_CNT;

    constexpr static map_node_t INIT = static_cast<map_node_t>(EnumBlocks::AIR) | FLAG_IS_LEAF;
    constexpr static map_node_t BIT_NO_LEAF = static_cast<map_node_t>(-1) ^ FLAG_IS_LEAF;
    constexpr static map_node_t BIT_NO_ALL_FULL = static_cast<map_node_t>(-1) ^ FLAG_IS_ALL_FULL;

    M_FORCE_INLINE MapNode(): childs_{INIT, INIT, INIT, INIT, INIT, INIT, INIT, INIT} {}
    M_FORCE_INLINE void Init() { for (int i = 0; i < 8; ++i) childs_[i] = INIT; }

    M_FORCE_INLINE MapNode *GetChildWithDiff(map_node_t diff)
    {
        return reinterpret_cast<MapNode *>(reinterpret_cast<unsigned char *>(this) + diff);
    }
    M_FORCE_INLINE const MapNode *GetChildWithDiff(map_node_t diff) const
    {
        return reinterpret_cast<const MapNode *>(reinterpret_cast<const unsigned char *>(this) + diff);
    }
    M_FORCE_INLINE MapNode *GetChildWithData(map_node_t child)
    {
        map_node_t diff = child & DIFF_BITS;
        if (child & FLAG_IS_FAR_PTR)
            return reinterpret_cast<MapNode *>(reinterpret_cast<unsigned char *>(this) + 
                                               far_diffs_[(uint32_t)(uint16_t)child >> FLAG_SHR_CNT]);
        return this->GetChildWithDiff(diff);
    }
    M_FORCE_INLINE const MapNode *GetChildWithData(map_node_t child) const
    {
        map_node_t diff = child & DIFF_BITS;
        if (child & FLAG_IS_FAR_PTR)
            return reinterpret_cast<const MapNode *>(reinterpret_cast<const unsigned char *>(this) + 
                                               far_diffs_[(uint32_t)(uint16_t)child >> FLAG_SHR_CNT]);
        return this->GetChildWithDiff(diff);
    }
    M_FORCE_INLINE MapNode *GetChildWithIndex(int index)
    {
        return GetChildWithData(childs_[index]);
    }
    M_FORCE_INLINE static constexpr game_block_t GetBlockID(map_node_t child)
    {
        map_node_t data = child >> FLAG_SHR_CNT;
        if (child & FLAG_IS_FAR_PTR)
            return static_cast<game_block_t>(far_diffs_[data]);
        return static_cast<game_block_t>(data);
    }

    friend class GameMap;
public:
};

static_assert(sizeof(MapNode) == 16);

}