#include "GameMap.h"

#include <type_traits>
#include <tuple>

#include "DynamicArray.h"
#include "StaticStack.h"
#include "MapGenerator.h"
#include "BlockListManager.h"

namespace vox::wrd
{

static_assert(sizeof(MapNode) == 16);
static_assert(alignof(MapNode) == 16);

void GameMap::Init()
{
    map_nodes_.Init(64);
    MapNode::far_diffs_.Init(64);
    map_node_recycle_diff_ = 0;
    map_far_diff_recycle_diff_ = 0;

    map_nodes_.PushBack_CapacityEnsured(MapNode{});
}

void GameMap::Clear()
{
    MapNode::far_diffs_.ClearStrict();
    map_nodes_.ClearStrict();
}

void GameMap::GenerateMap(seed_t seed)
{
    rand_engine_.Seed(seed);
    map_nodes_.Clear();
    MapNode::far_diffs_.Clear();
    map_nodes_.PushBack_CapacityEnsured(MapNode{});
    
    wrd::MGGenerateMap(rand_engine_);
}

void GameMap::LoadFromFile(FILE* fp)
{
    rand_engine_.LoadFromFile(fp);
    map_nodes_.LoadFromFile(fp);
    MapNode::far_diffs_.LoadFromFile(fp);
}

void GameMap::SaveToFile(FILE* fp) const
{
    rand_engine_.SaveToFile(fp);
    map_nodes_.SaveToFile(fp);
    MapNode::far_diffs_.SaveToFile(fp);
}

MapNode::map_node_t GameMap::AddMapNode(MapNode** p_new_node, MapNode *cur_pos)
{
    assert(&map_nodes_[0] <= cur_pos);
    assert(cur_pos < map_nodes_.GetDataEndPtr());

    MapNode *new_node;
    if (map_node_recycle_diff_ == 0)
    {
        const size_t cur_diff = cur_pos - &map_nodes_[0];
        map_nodes_.PushBack(MapNode{});
        new_node = map_nodes_.GetDataEndPtr() - 1;
        cur_pos = &map_nodes_[0] + cur_diff;

        assert(&map_nodes_[0] <= new_node);
        assert(new_node < map_nodes_.GetDataEndPtr());
        assert(&map_nodes_[0] <= cur_pos);
        assert(cur_pos < map_nodes_.GetDataEndPtr());
    }
    else
    {
        new_node = &map_nodes_[0] + map_node_recycle_diff_;
        map_node_recycle_diff_ = *(size_t *)new_node;
        new_node->Init();

        assert(&map_nodes_[0] <= new_node);
        assert(new_node < map_nodes_.GetDataEndPtr());
        assert(map_node_recycle_diff_ < map_nodes_.GetSize());
    }
    *p_new_node = new_node;

    int64_t diff = (unsigned char*)new_node - (unsigned char*)cur_pos;

    assert(diff % 16 == 0);
    assert(diff / 16 < map_nodes_.GetSize());

    if (std::abs(diff) <= MapNode::MAX_SHORT_DIFF)
    {
        return diff;
    }
    else
    {
        return (AddFarDiff(diff) << MapNode::FLAG_SHR_CNT) | MapNode::FLAG_IS_FAR_PTR;
    }
}

void GameMap::FreeMapNode(MapNode *node)
{
    *(size_t *)node = map_node_recycle_diff_;
    map_node_recycle_diff_ = node - &map_nodes_[0];
}

int32_t GameMap::AddFarDiff(int32_t data)
{
    assert(data / 16 < map_nodes_.GetSize());

    if (map_far_diff_recycle_diff_ == 0)
    {
        const size_t ret = MapNode::far_diffs_.GetSize();
        MapNode::far_diffs_.PushBack(data);

        assert(ret < MapNode::MAX_SHORT_BLOCK_DIFF);

        return ret;
    }
    else
    {
        assert(map_far_diff_recycle_diff_ < MapNode::far_diffs_.GetSize());

        const int32_t ret = map_far_diff_recycle_diff_;
        map_far_diff_recycle_diff_ = MapNode::far_diffs_[map_far_diff_recycle_diff_];
        MapNode::far_diffs_[ret] = data;

        assert(map_far_diff_recycle_diff_ < MapNode::far_diffs_.GetSize());
        assert(ret < MapNode::MAX_SHORT_BLOCK_DIFF);

        return ret;
    }
}

void GameMap::FreeFarDiff(int index)
{
    assert(index >= 0);
    assert(index < MapNode::far_diffs_.GetSize());

    MapNode::far_diffs_[index] = map_far_diff_recycle_diff_;
    map_far_diff_recycle_diff_ = index;

    assert(map_far_diff_recycle_diff_ < MapNode::far_diffs_.GetSize());
}

void GameMap::SetBlock(EnumBlocks block_id_e, map_size_t block_sz_log, map_size_t y, map_size_t z, map_size_t x)
{
    assert(map_nodes_.GetSize() > 0);

    const game_block_t block_id = (game_block_t)block_id_e;
    data::StaticStack<size_t, MAP_OCTREE_DEPTH> node_stack{};

    MapNode *cur_node = &map_nodes_[0];
    node_stack.PushBack(cur_node - &map_nodes_[0]);
    const unsigned blk_sz = 1 << block_sz_log;
    for (auto cur_blk_sz_log = MAP_OCTREE_DEPTH - 1; cur_blk_sz_log > block_sz_log; --cur_blk_sz_log)
    {
        const unsigned cur_blk_sz = 1 << cur_blk_sz_log;

        const unsigned node_pos_y = (y & cur_blk_sz) >> cur_blk_sz_log;
        const unsigned node_pos_z = (z & cur_blk_sz) >> cur_blk_sz_log;
        const unsigned node_pos_x = (x & cur_blk_sz) >> cur_blk_sz_log;
        const unsigned node_pos = node_pos_y << 2 | node_pos_z << 1 | node_pos_x;

        assert(0 <= node_pos);
        assert(node_pos < 8);

        const auto prev_data = cur_node->childs_[node_pos];
        if (prev_data & MapNode::FLAG_IS_LEAF)
        {
            MapNode *new_node;
            const size_t diff_cur_node = cur_node - &map_nodes_[0];


            assert(&map_nodes_[0] <= cur_node);
            assert(cur_node < map_nodes_.GetDataEndPtr());
            assert(diff_cur_node < map_nodes_.GetSize());

            auto new_data = (AddMapNode(&new_node, cur_node) | (prev_data & MapNode::FLAG_BITS)) & MapNode::BIT_NO_LEAF;
            if (block_id_e == EnumBlocks::AIR)
                new_data &= MapNode::BIT_NO_ALL_FULL;
            else
                new_data |= MapNode::FLAG_IS_SOMEADJ_FULL;

            cur_node = &map_nodes_[0] + diff_cur_node;

            assert(&map_nodes_[0] <= cur_node);
            assert(cur_node < map_nodes_.GetDataEndPtr());
            assert(&map_nodes_[0] <= new_node);
            assert(new_node < map_nodes_.GetDataEndPtr());
            assert(new_node - &map_nodes_[0] < map_nodes_.GetSize());

            cur_node->childs_[node_pos] = new_data;
            new_node->childs_[0] = prev_data;
            new_node->childs_[1] = prev_data;
            new_node->childs_[2] = prev_data;
            new_node->childs_[3] = prev_data;
            new_node->childs_[4] = prev_data;
            new_node->childs_[5] = prev_data;
            new_node->childs_[6] = prev_data;
            new_node->childs_[7] = prev_data;

            cur_node = new_node;
        }
        else
        {
            MapNode *const new_node = cur_node->GetChildWithData(prev_data);

            size_t diff_new_node = new_node - &map_nodes_[0];
            assert(&map_nodes_[0] <= cur_node);
            assert(cur_node < map_nodes_.GetDataEndPtr());
            assert(&map_nodes_[0] <= new_node);
            assert(new_node < map_nodes_.GetDataEndPtr());
            assert(diff_new_node < map_nodes_.GetSize());

            cur_node = new_node;
        }
        node_stack.PushBack(cur_node - &map_nodes_[0]);
    }

    const unsigned node_pos_y = (y & blk_sz) >> block_sz_log;
    const unsigned node_pos_z = (z & blk_sz) >> block_sz_log;
    const unsigned node_pos_x = (x & blk_sz) >> block_sz_log;
    const unsigned node_pos = node_pos_y << 2 | node_pos_z << 1 | node_pos_x;

    assert(0 <= node_pos);
    assert(node_pos < 8);

    const auto full_flag = block_id_e == EnumBlocks::AIR ?
        MapNode::FLAG_IS_LEAF :
        MapNode::FLAG_IS_LEAF | MapNode::FLAG_IS_ALL_FULL | MapNode::FLAG_IS_SOMEADJ_FULL;

    if (block_id <= MapNode::MAX_SHORT_BLOCK_DIFF)
    {
        cur_node->childs_[node_pos] = 
            (block_id << MapNode::FLAG_SHR_CNT) | full_flag;
    }
    else
    {
        // TODO not yet prepared and tested
        assert(0);

        static_assert(std::is_same_v<decltype(MapNode::far_diffs_.Get(0)), int32_t&>);

        const auto sz = MapNode::far_diffs_.GetSize();
        MapNode::far_diffs_.PushBack((int32_t)block_id);

        cur_node->childs_[node_pos] =
            sz << MapNode::FLAG_SHR_CNT | full_flag;
    }

    // back track nodes to combine same nodes
    for (auto cur_blk_sz_log = block_sz_log + 1; cur_blk_sz_log < MAP_OCTREE_DEPTH; ++cur_blk_sz_log)
    {
        const auto cur_blk_sz = 1 << cur_blk_sz_log;

        const auto cur_node = node_stack.GetBack() + &map_nodes_[0];
        node_stack.PopBack();

        const auto node_data = cur_node->childs_[0];
        if (!(node_data & MapNode::FLAG_IS_LEAF))
        {
            goto DONT_COMBINE;
        }
        for (int i = 1; i < 8; ++i)
        {
            if (cur_node->childs_[i] != node_data)
                goto DONT_COMBINE;
        }

        // combine
        // TODO this dont consider far node leaf
        {
            const auto node_pos_y = (y & cur_blk_sz) >> cur_blk_sz_log;
            const auto node_pos_z = (z & cur_blk_sz) >> cur_blk_sz_log;
            const auto node_pos_x = (x & cur_blk_sz) >> cur_blk_sz_log;
            const auto node_pos = node_pos_y << 2 | node_pos_z << 1 | node_pos_x;

            assert(0 <= node_pos);
            assert(node_pos < 8);

            auto& par = (node_stack.GetBack() + &map_nodes_[0])->childs_[node_pos];
            if (par & MapNode::FLAG_IS_FAR_PTR)
            {
                FreeFarDiff((uint32_t)(uint16_t)par >> MapNode::FLAG_SHR_CNT);
            }
            par = node_data;
            FreeMapNode(cur_node);
        }

        continue;
    DONT_COMBINE:
        for (int i = 0; i < 8; ++i)
        {
            if (!(cur_node->childs_[i] & MapNode::FLAG_IS_ALL_FULL))
                goto END_COMBINE;
        }

        {
            const auto node_pos_y = (y & cur_blk_sz) >> cur_blk_sz_log;
            const auto node_pos_z = (z & cur_blk_sz) >> cur_blk_sz_log;
            const auto node_pos_x = (x & cur_blk_sz) >> cur_blk_sz_log;
            const auto node_pos = node_pos_y << 2 | node_pos_z << 1 | node_pos_x;

            assert(0 <= node_pos);
            assert(node_pos < 8);

            (node_stack.GetBack() + &map_nodes_[0])->childs_[node_pos] |= MapNode::FLAG_IS_ALL_FULL;
        }

    END_COMBINE:;
    }
}


size_t GameMap::GenerateBlockInfo(void* dest, map_size_t block_sz_log, map_size_t y, map_size_t z, map_size_t x) const
{
    auto &data = *(data::DynamicArray<ren::BLockListData> *)dest;
    data.Clear();

    const MapNode *cur_node = &map_nodes_[0];
    const unsigned blk_sz = 1 << block_sz_log;
    // head down to node in chunk
    for (auto cur_blk_sz_log = MAP_OCTREE_DEPTH - 1; cur_blk_sz_log > block_sz_log; --cur_blk_sz_log)
    {

        const unsigned cur_blk_sz = 1 << cur_blk_sz_log;

        const unsigned node_pos_y = (y & cur_blk_sz) >> cur_blk_sz_log;
        const unsigned node_pos_z = (z & cur_blk_sz) >> cur_blk_sz_log;
        const unsigned node_pos_x = (x & cur_blk_sz) >> cur_blk_sz_log;
        const unsigned node_pos = node_pos_y << 2 | node_pos_z << 1 | node_pos_x;

        const auto prev_data = cur_node->childs_[node_pos];
        if (prev_data & MapNode::FLAG_IS_LEAF)
        {
            // TODO consider far pointer leaf node
            // TODO consider multiblock

            game_block_t block_type = prev_data >> MapNode::FLAG_SHR_CNT;
            if (block_type == (game_block_t)EnumBlocks::AIR)
                return 0;

            data.PushBack_CapacityEnsured(ren::BLockListData{
                (uint16_t)y, (uint16_t)z, (uint16_t)x, (uint16_t)(block_type << MapNode::FLAG_SHR_CNT | block_sz_log)});
            return 1;
        }
        else
        {
            const MapNode *const new_node = cur_node->GetChildWithData(prev_data);

            size_t diff_new_node = new_node - &map_nodes_[0];
            assert(&map_nodes_[0] <= cur_node);
            assert(cur_node < map_nodes_.GetDataEndPtr());
            assert(&map_nodes_[0] <= new_node);
            assert(new_node < map_nodes_.GetDataEndPtr());
            assert(diff_new_node < map_nodes_.GetSize());

            cur_node = new_node;
        }
    }

    // generate block infos
    data::StaticStack<std::tuple<const MapNode *, uint16_t, uint16_t, uint16_t, uint16_t>, (MAP_OCTREE_DEPTH + 2) * 8> node_stack{};
    node_stack.PushBack(std::make_tuple(cur_node, y, z, x, block_sz_log));
    while (!node_stack.IsEmpty())
    {
        const auto tup = node_stack.GetBack();
        const MapNode* const top_node = std::get<0>(tup);
        node_stack.PopBack();
        const auto cur_blk_sz = std::get<4>(tup);

        assert(&map_nodes_[0] <= top_node);
        assert(top_node < map_nodes_.GetDataEndPtr());

        for (int i = 0; i < 8; ++i)
        {
            auto o = top_node->childs_[i];

            if (o & MapNode::FLAG_IS_LEAF)
            {
                // TODO consider far pointer leaf node
                // TODO consider multiblock
                // TODO check AIR
                // TOOD cull sealed block

                game_block_t block_type = o >> MapNode::FLAG_SHR_CNT;
                if (block_type == (game_block_t)EnumBlocks::AIR)
                    continue;

                data.PushBack(ren::BLockListData{
                    std::get<1>(tup), std::get<2>(tup), std::get<3>(tup),
                        (uint16_t)(o & MapNode::DIFF_BITS | cur_blk_sz)});
            }
            else
            {
                const MapNode *child_node = top_node->GetChildWithData(o);

                assert(&map_nodes_[0] <= child_node);
                assert(child_node < map_nodes_.GetDataEndPtr());


                node_stack.PushBack(std::make_tuple(
                    child_node,
                    std::get<1>(tup) | (i & 4) >> 2 << cur_blk_sz,
                    std::get<2>(tup) | (i & 2) >> 1 << cur_blk_sz,
                    std::get<3>(tup) | (i & 1) << cur_blk_sz,
                    cur_blk_sz - 1));
            }
        }
    }

    return (size_t)data.GetSize();
}

}
