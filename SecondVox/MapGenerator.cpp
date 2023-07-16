#include "MapGenerator.h"

#include <random>

#include "GameMap.h"
#include "MapManager.h"

namespace vox::wrd
{

void MGGenerateMap(utils::LinearRandomEngine &rand)
{
    auto& gm = MMGetGameMap();
    std::uniform_int_distribution<int> dist(0, 10);

    const auto md = GameMap::MAP_SZ >> 6;
    for (int zz = md; zz < md * 2; ++zz)
    for (int xx = md; xx < md * 2; ++xx)
    {
        const auto yyy = 20 + dist(rand);
        for (int yy = 0; yy < yyy; ++yy)
        {
            gm.SetBlock(EnumBlocks::TEMP, 0, yy, zz, xx);
        }
    }
}

}
