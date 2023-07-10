#pragma once

#include <cstdint>

#include "Macros.h"
#include "UtilsRandom.h"
#include "UtilsSave.h"
#include "GameBlock.h"

namespace vox::wrd
{

class GameMap;

struct alignas(16) MapNode
{
public:
    using map_node_t = int16_t;
    map_node_t childs[8];
    static int32_t *far_diffs;

    constexpr static map_node_t FLAG_BITS = 0b1111;
    constexpr static map_node_t DIFF_BITS = (map_node_t)-1 ^ FLAG_BITS;
    constexpr static map_node_t FLAG_SHR_CNT = 4;

    constexpr static map_node_t FLAG_IS_ALL_FULL = 0b1;
    constexpr static map_node_t FLAG_IS_SOMEADJ_FULL = 0b10;
    constexpr static map_node_t FLAG_IS_FAR_PTR = 0b100;
    constexpr static map_node_t FLAG_IS_LEAF = 0b1000;

    M_FORCE_INLINE MapNode *GetChildWithDiff(map_node_t diff)
    {
        return reinterpret_cast<MapNode *>(reinterpret_cast<unsigned char *>(this) + diff);
    }
    M_FORCE_INLINE MapNode *GetChild(map_node_t child)
    {
        map_node_t diff = child & DIFF_BITS;
        if (child & FLAG_IS_FAR_PTR)
            diff = far_diffs[child >> FLAG_SHR_CNT];
        return this->GetChildWithDiff(diff);
    }
    M_FORCE_INLINE static constexpr game_block_t GetBlockID(map_node_t child)
    {
        return static_cast<game_block_t>(child);
    }

};
static_assert(sizeof(MapNode) == 16);
static_assert(alignof(MapNode) == 16);

using map_size_t = int32_t;

class GameMap
{
private:
    size_t map_nodes_count_;
    size_t map_nodes_capacity_;
    MapNode* map_nodes_;

    size_t edge_size_;
public:
    M_FORCE_INLINE size_t GetMapNodesCount() const { return map_nodes_count_; }
    M_FORCE_INLINE const MapNode *GetMapNodeRawPTR() const { return map_nodes_; }
    void LoadFromFile(FILE* fp);
    void SaveToFile(FILE* fp) const;
    void Init(size_t edge_size);
    void Clear();

    void InsertBlock(game_block_t block_id, map_size_t block_sz_log, map_size_t y, map_size_t z, map_size_t x);
};


}

