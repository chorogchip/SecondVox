#include "GameMap.h"

#include <iostream>


namespace vox::wrd
{

void GameMap::LoadFromFile(FILE* fp)
{
    fread(&map_nodes_count_, sizeof(map_nodes_count_), 1, fp);
    map_nodes_capacity_ = map_nodes_count_;
    if (map_nodes_ != nullptr)
    {
        _aligned_free(map_nodes_);
        map_nodes_ = nullptr;
    }
    map_nodes_ = (MapNode*)_aligned_malloc(sizeof(MapNode) * map_nodes_capacity_, alignof(MapNode));
    if (map_nodes_ == nullptr)
    {
        std::cout << "error load map from file" << std::endl;
        __debugbreak();
    }
    fread(map_nodes_, sizeof(MapNode), map_nodes_count_, fp);
}

void GameMap::SaveToFile(FILE* fp) const
{
    fwrite(&map_nodes_count_, sizeof(map_nodes_count_), 1, fp);
    fwrite(map_nodes_, sizeof(MapNode), map_nodes_count_, fp);
}

void GameMap::Init(size_t edge_size)
{
    edge_size_ = edge_size;
}

void GameMap::Clear()
{
    if (map_nodes_ != nullptr)
    {
        _aligned_free(map_nodes_);
        map_nodes_ = nullptr;
    }
}


void GameMap::InsertBlock(game_block_t block_id, map_size_t block_sz_log, map_size_t y, map_size_t z, map_size_t x)
{
    
}

}
