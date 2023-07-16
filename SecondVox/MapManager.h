#pragma once

#include "UtilsRandom.h"

#include "GameMap.h"

namespace vox::wrd
{

struct MapDesc
{
    int32_t map_id;
    int portal_count;
};

void MMInit(seed_t seed);
void MMClear();
void MMChangeMap(const MapDesc* p_desc);
void MMSaveMap();
GameMap &MMGetGameMap();

}
