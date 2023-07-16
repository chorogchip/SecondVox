#include "MapManager.h"

#include <vector>
#include <iostream>
#include <memory>

#include "Macros.h"
#include "UtilsRandom.h"

#include "GameMap.h"

namespace vox::wrd
{

static MapDesc map_desc_;
static GameMap game_map_;
static seed_t seed_;

void MMInit(seed_t seed)
{
    seed_ = seed;
    map_desc_.map_id = -1;

    game_map_.Init();
}

void MMClear()
{
    MMSaveMap();
    game_map_.Clear();
}

void MMChangeMap(const MapDesc* p_desc)
{
    MMSaveMap();
    memcpy(&map_desc_, p_desc, sizeof(map_desc_));

    char buf[256];
    sprintf_s(buf, "GameData/World/Maps/map%d", map_desc_.map_id);
    FILE* fp;
    fopen_s(&fp, buf, "wb");

    // force to generate map
    if(fp != nullptr) { fclose(fp); } fp = nullptr;

    if (fp == nullptr)
    {
        game_map_.GenerateMap(utils::GenerateSeedBySeed(seed_ ^ p_desc->map_id));
        return;
    }

    //fread(&map_info_, sizeof(map_info_), 1, fp);
    game_map_.LoadFromFile(fp);

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

    //fwrite(&map_info_, sizeof(map_info_), 1, fp);
    game_map_.SaveToFile(fp);

    fclose(fp);
}

GameMap &MMGetGameMap()
{
    return game_map_;
}

}