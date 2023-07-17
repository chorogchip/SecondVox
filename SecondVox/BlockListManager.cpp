#include "BlockListManager.h"

#include "GlobalMemories.h"
#include "MapManager.h"

namespace vox::ren
{

// y:16, z:16, x:16, mat:12, size:4 

static constexpr auto CHNK_AXIS = wrd::GameMap::CHUNK_CNT_1AXIS;
static constexpr auto CHNK_SZ = wrd::GameMap::CHUNK_SZ;
static constexpr auto CHNK_SZ_LOG = wrd::GameMap::CHUNK_SZ_LOG;

struct ChunkBlockInfo
{
    bool to_update_;
} chunk_block_infos_[CHNK_AXIS * CHNK_AXIS * CHNK_AXIS];
static bool global_to_update_ = false;

void BMInit()
{
    global_to_update_ = true;
    for (auto& o : chunk_block_infos_)
    {
        o.to_update_ = true;
    }
}

void BMClear()
{

    for (auto& o : chunk_block_infos_)
    {
        // clear GPU mem
    }
}

void BMMarkUpdate(wrd::map_size_t y, wrd::map_size_t z, wrd::map_size_t x)
{
    global_to_update_ = true;
    chunk_block_infos_[y * CHNK_AXIS * CHNK_AXIS + z * CHNK_AXIS + x].to_update_ = true;
}

void BMUpdate()
{
    if (!global_to_update_)
        return;

    global_to_update_ = false;

    auto& gm = wrd::MMGetGameMap();
    auto& data = mem::GMGetData(mem::EnumGLobalMemories::BLOCK_LIST_GEN).ClearAndCollapse<BLockListData>();

    for (int yy = 0; yy < CHNK_AXIS; ++yy)
        for (int zz = 0; zz < CHNK_AXIS; ++zz)
            for (int xx = 0; xx < CHNK_AXIS; ++xx)
            {
                auto& chk = chunk_block_infos_[yy * CHNK_AXIS * CHNK_AXIS + zz * CHNK_AXIS + xx];
                if (!chk.to_update_)
                    continue;
                chk.to_update_ = false;
                
                data.Clear();

                const auto size = gm.GenerateBlockInfo(&data, CHNK_SZ_LOG, yy * CHNK_SZ, zz * CHNK_SZ, xx * CHNK_SZ);
                if (size == 0)
                    continue;

                // send to GPU

            }

    mem::GMFreeData(mem::EnumGLobalMemories::BLOCK_LIST_GEN);
}

}