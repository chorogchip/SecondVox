#pragma once

#include <cstdint>

#include "Macros.h"
#include "UtilsRandom.h"
#include "UtilsSave.h"
#include "UtilsMath.h"
#include "DynamicArray.h"

#include "GameBlock.h"
#include "MapNode.h"

namespace vox::wrd
{

using map_size_t = int32_t;

class GameMap
{
private:
    data::DynamicArray<MapNode> map_nodes_;
    utils::LinearRandomEngine rand_engine_;
    size_t map_node_recycle_diff_;
    size_t map_far_diff_recycle_diff_;

    MapNode::map_node_t AddMapNode(MapNode** p_new_node, MapNode *cur_pos);
    void FreeMapNode(MapNode *node);
    int32_t AddFarDiff(int32_t data);
    void FreeFarDiff(int index);

public:
    static constexpr map_size_t LARGE_BLOCK_CNT_1AXIS = 1024;
    static constexpr map_size_t LARGE_BLOCK_SZ = 8;
    static constexpr map_size_t MAP_SZ = LARGE_BLOCK_CNT_1AXIS * LARGE_BLOCK_SZ;
    static constexpr map_size_t MAP_OCTREE_DEPTH = 13;
    static constexpr map_size_t CHUNK_SZ = 64 * LARGE_BLOCK_SZ;
    static constexpr map_size_t CHUNK_SZ_LOG = 6 + 3;
    static constexpr map_size_t CHUNK_CNT_1AXIS = MAP_SZ / CHUNK_SZ;

    static_assert(1 << MAP_OCTREE_DEPTH == MAP_SZ);
    static_assert(1 << CHUNK_SZ_LOG == CHUNK_SZ);

    void Init();
    void Clear();
    void GenerateMap(seed_t seed);
    void LoadFromFile(FILE* fp);
    void SaveToFile(FILE* fp) const;

    void SetBlock(EnumBlocks block_id, map_size_t block_sz_log, map_size_t y, map_size_t z, map_size_t x);
    size_t GenerateBlockInfo(void* dest, map_size_t block_sz_log, map_size_t y, map_size_t z, map_size_t x) const;
};


}

