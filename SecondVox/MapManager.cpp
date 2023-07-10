#include "MapManager.h"

#include <vector>
#include <iostream>
#include <memory>

#include "Macros.h"
#include "UtilsRandom.h"

#include "GameMap.h"

namespace vox::wrd
{

static seed_t seed_;
static MapDesc map_desc_;

static struct MapInfo
{
    seed_t seed;

} map_info_;

static GameMap game_map;



static void GenerateMap();

void MMInit(seed_t seed)
{
    seed_ = utils::GenerateSeedBySeed(seed, utils::EnumSeedsForInit::MAP_MANAGER);
    map_desc_.map_id = -1;
}

void MMClear()
{
    MMSaveMap();
    game_map.Clear();
}

void MMChangeMap(const MapDesc* p_desc)
{
    MMSaveMap();
    memcpy(&map_desc_, p_desc, sizeof(map_desc_));

    char buf[256];
    sprintf_s(buf, "GameData/World/Maps/map%d", map_desc_.map_id);
    FILE* fp;
    fopen_s(&fp, buf, "wb");
    if (fp == nullptr)
    {
        GenerateMap();
        return;
    }

    fread(&map_info_, sizeof(map_info_), 1, fp);
    game_map.LoadFromFile(fp);

    fclose(fp);
}

void MMSaveMap()
{
    if (map_desc_.map_id == -1)
        return;

    char buf[256];
    sprintf_s(buf, "GameData/World/Maps/map%d", map_desc_.map_id);
    FILE* fp;
    fopen_s(&fp, buf, "wb");
    if (fp == nullptr)
    {
        std::cout << "failed to open map file: " << buf << std::endl;
        return;
    }

    fwrite(&map_info_, sizeof(map_info_), 1, fp);
    game_map.SaveToFile(fp);

    fclose(fp);
}

static void GenerateMap()
{
    map_info_.seed = utils::GenerateSeedBySeed(seed_ + (seed_t)map_desc_.map_id, utils::EnumSeedsForInit::ORIGINAL);
    
}

}