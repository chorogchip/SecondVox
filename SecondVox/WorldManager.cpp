#include "WorldManager.h"

#include <vector>
#include <iostream>

#include "UtilsRandom.h"
#include "MapManager.h"

namespace vox::wrd
{

static constexpr int MIN_STRONG_PORTAL_COUNT_PER_MAP = 1;
static constexpr int MAX_STRONG_PORTAL_COUNT_PER_MAP = 2;
static constexpr int MIN_WEAK_PORTAL_COUNT_PER_MAP = 0;
static constexpr int MAX_WEAK_PORTAL_COUNT_PER_MAP = 2;

static constexpr int MAX_PORTAL_COUNT_PER_MAP =
    MAX_STRONG_PORTAL_COUNT_PER_MAP + MAX_WEAK_PORTAL_COUNT_PER_MAP;

using portal_id_t = int32_t;
using map_id_t = int32_t;



static seed_t seed_ = 0;
static utils::LinearRandomEngine rand_engine_;
static std::uniform_int_distribution<int> strong_portal_dist(
    MIN_STRONG_PORTAL_COUNT_PER_MAP, MAX_STRONG_PORTAL_COUNT_PER_MAP);
static std::uniform_int_distribution<int> weak_portal_dist(
    MIN_WEAK_PORTAL_COUNT_PER_MAP, MAX_WEAK_PORTAL_COUNT_PER_MAP);
static std::uniform_int_distribution<seed_t> map_seed_dist{};
static std::uniform_int_distribution<int> weak_portal_connect_dist(1, 3); 




struct PortalInfo
{
private:
    bool is_other_side_initialized_ = false;
    bool is_strong_portal_ = false;
    map_id_t connected_maps_[2] = {};
public:
    void Init(map_id_t from_map, bool is_strong_portal);
    map_id_t GetConnectedMap(map_id_t start_map);
};

struct MapInfo
{
private:
    int strong_portal_count_;
    int weak_portal_count_;
    int cur_weak_portal_count_;
    portal_id_t parent_strong_weak_portals_[MAX_PORTAL_COUNT_PER_MAP + 1];
    bool is_weak_portal_initialized_[MAX_WEAK_PORTAL_COUNT_PER_MAP];
    seed_t seed_;
public:
    void Init(portal_id_t parent_portal);
    portal_id_t GetPortal(int portal_num_of_map);
    M_FORCE_INLINE auto GetStrongPortalCount() const { return strong_portal_count_; }
    M_FORCE_INLINE auto GetPortalCount() const { return strong_portal_count_ + weak_portal_count_ + 1; }
    bool ConnectWeakPortal(portal_id_t portal_id);
};

static std::vector<PortalInfo> portal_infos_;
static std::vector<MapInfo> map_infos_;




void PortalInfo::Init(map_id_t from_map, bool is_strong_portal)
{
    is_other_side_initialized_ = false;
    is_strong_portal_ = is_strong_portal;
    connected_maps_[0] = from_map;
}

map_id_t PortalInfo::GetConnectedMap(map_id_t start_map)
{
    if (is_other_side_initialized_)
        return connected_maps_[connected_maps_[0] == start_map];

    if (is_strong_portal_)
    {
        map_id_t new_map_id = static_cast<map_id_t>(map_infos_.size());
        connected_maps_[1] = new_map_id;
        is_other_side_initialized_ = true;
        map_infos_.emplace_back();
        map_infos_[new_map_id].Init(this - &portal_infos_[0]);
    }
    else
    {
        map_id_t new_map_id = 0;
        // complex weak portal connecting algorithm
        {
            const int travel_count = weak_portal_connect_dist(rand_engine_);
            map_id_t cur_map = start_map;
            std::vector<map_id_t> map_stack{};
            map_stack.reserve(travel_count);
            for (int i = 0; i < travel_count; ++i)
            {
                // go to parent
                cur_map = portal_infos_[map_infos_[cur_map].GetPortal(0)].GetConnectedMap(cur_map);
            }
            for (int i = 0; i < travel_count; ++i)
            {
                map_stack.push_back(cur_map);
                const int cnt = map_infos_[cur_map].GetStrongPortalCount();
                std::uniform_int_distribution<int> strong_portal_dist(0, cnt - 1);
                const int strong_portal_num = strong_portal_dist(rand_engine_);
                cur_map = portal_infos_[map_infos_[cur_map].GetPortal(cnt + strong_portal_num)].GetConnectedMap(cur_map);
            }

            portal_id_t this_portal = this - &portal_infos_[0];
            if (cur_map != start_map && map_infos_[cur_map].ConnectWeakPortal(this_portal))
            {
                goto GO_TO_PORTAL;
            }

            while (!map_stack.empty())
            {
                cur_map = map_stack.back();
                map_stack.pop_back();

                if (map_infos_[cur_map].ConnectWeakPortal(this_portal))
                {
                    new_map_id = cur_map;
                    goto GO_TO_PORTAL;
                }
            }

            // could not find portal to connect
            {
                new_map_id = 0;
                // or maybe able to traverse infitite to children?
            }
        }

        GO_TO_PORTAL:
        connected_maps_[1] = new_map_id;
        is_other_side_initialized_ = true;
    }

    return connected_maps_[1];
}

void MapInfo::Init(portal_id_t parent_portal)
{
    strong_portal_count_ = strong_portal_dist(rand_engine_);
    weak_portal_count_ = weak_portal_dist(rand_engine_);
    cur_weak_portal_count_ = 0;
    seed_ = map_seed_dist(rand_engine_);


    // make only parent portal and strong portals, not weak portals

    int ind = 0;

    parent_strong_weak_portals_[ind++] = parent_portal;

    for (int i = 0; i < strong_portal_count_; ++i)
    {
        portal_id_t new_portal = static_cast<portal_id_t>(portal_infos_.size());
        parent_strong_weak_portals_[ind++] = new_portal;
        portal_infos_.emplace_back();
        portal_infos_[new_portal].Init(this - &map_infos_[0], true);
    }

    memset(is_weak_portal_initialized_, 0, sizeof(is_weak_portal_initialized_));
}

portal_id_t MapInfo::GetPortal(int portal_num_of_map)
{
    if (portal_num_of_map < 0 ||
        portal_num_of_map >= 1 + strong_portal_count_ + weak_portal_count_)
    {
        std::cout << "wrong portal num: " << portal_num_of_map << std::endl;
        return 0;
    }

    if (portal_num_of_map >= 1 + strong_portal_count_)
    {
        // weak portal
        const int weak_offset = 1 + strong_portal_count_;
        const int index = portal_num_of_map - weak_offset;
        if (!is_weak_portal_initialized_[index])
        {
            // weak portal not initialized

            portal_id_t new_portal = static_cast<portal_id_t>(portal_infos_.size());
            parent_strong_weak_portals_[index + weak_offset] = new_portal;
            is_weak_portal_initialized_[index] = true;
            portal_infos_.emplace_back();
            portal_infos_[new_portal].Init(this - &map_infos_[0], true);
        }
    }
    
    return parent_strong_weak_portals_[portal_num_of_map];
}

bool MapInfo::ConnectWeakPortal(portal_id_t portal_id)
{
    if (cur_weak_portal_count_ == weak_portal_count_)
        return false;

    for (int i = 0; i < weak_portal_count_; ++i)
        if (!is_weak_portal_initialized_[i])
        {
            is_weak_portal_initialized_[i] = true;
            weak_portal_count_++;
            parent_strong_weak_portals_[i + strong_portal_count_ + 1] = portal_id;
            return true;
        }

    assert(0);
    return false;
}

static map_id_t current_map_ = 0;
static map_id_t map_to_change_ = 0;
static bool to_change_map_ = false;

struct WorldFileV0
{
    seed_t seed;
    int portal_count;
    int map_count;
    map_id_t cur_map;
};

static constexpr int32_t WORLD_MANAGER_VERSION = 0;
static constexpr auto WORLD_FILE_NAME = "GameData/World/world";

void WMInit()
{
    // clear
    current_map_ = 0;
    map_to_change_ = 0;
    to_change_map_ = false;
    map_infos_.clear();
    portal_infos_.clear();

    FILE* fp;
    fopen_s(&fp, WORLD_FILE_NAME, "rb");
    if (fp == nullptr)
    {
        seed_ = utils::GenerateSeedByTime();
        rand_engine_  = utils::LinearRandomEngine(seed_);

        map_infos_.emplace_back();
        map_infos_[0].Init(0);
        current_map_ = 0;

        goto END_INIT;
    }

    int32_t world_manager_version;
    fread(&world_manager_version, sizeof(world_manager_version), 1, fp);
    if (world_manager_version == 0)
    {
        WorldFileV0 file_world_data;
        fread(&file_world_data, sizeof(file_world_data), 1, fp);
        seed_ = file_world_data.seed;
        portal_infos_.reserve(file_world_data.portal_count);
        map_infos_.reserve(file_world_data.map_count);
        current_map_ = file_world_data.cur_map;

        for (int i = 0; i < file_world_data.portal_count; ++i)
        {
            PortalInfo file_portal_data;
            fread(&file_portal_data, sizeof(file_portal_data), 1, fp);
            portal_infos_.push_back(file_portal_data);
        }

        for (int i = 0; i < file_world_data.map_count; ++i)
        {
            MapInfo file_map_data;
            fread(&file_map_data, sizeof(file_map_data), 1, fp);
            map_infos_.push_back(file_map_data);
        }

        fclose(fp);
        goto END_INIT;
    }
    else
    {
        std::cout << "wrong world manager verion in file " << WORLD_FILE_NAME << std::endl;
        return;
    }

END_INIT:
    MMInit(seed_);
    MapDesc md;
    md.map_id = current_map_;
    md.portal_count = map_infos_[md.map_id].GetPortalCount();
    MMChangeMap(&md);
}

void WMClear()
{
    MMClear();

    FILE* fp;
    fopen_s(&fp, WORLD_FILE_NAME, "wb");
    if (fp == nullptr)
    {
        std::cout << "error storing world file " << WORLD_FILE_NAME << std::endl;
        return;
    }
    fwrite(&WORLD_MANAGER_VERSION, sizeof(WORLD_MANAGER_VERSION), 1, fp);

    WorldFileV0 file_world_info;
    file_world_info.seed = seed_;
    file_world_info.portal_count = portal_infos_.size();
    file_world_info.map_count = map_infos_.size();
    file_world_info.cur_map = current_map_;
    fwrite(&file_world_info, sizeof(file_world_info), 1, fp);
    fwrite(&portal_infos_[0], sizeof(portal_infos_[0]), file_world_info.portal_count, fp);
    fwrite(&map_infos_[0], sizeof(map_infos_[0]), file_world_info.map_count, fp);

    fclose(fp);
}

void WMGoThroughPortalAndPrepareChangeMap(int portal_num_of_map)
{
    const portal_id_t in_portal = map_infos_[current_map_].GetPortal(portal_num_of_map);
    map_to_change_ = portal_infos_[in_portal].GetConnectedMap(current_map_);
    to_change_map_ = true;
}

void WMCheckToChangeMap()
{
    if (!to_change_map_)
        return;

    MapDesc md;
    md.map_id = map_to_change_;
    md.portal_count = map_infos_[map_to_change_].GetPortalCount();
    MMChangeMap(&md);
    
    current_map_ = map_to_change_;
    to_change_map_ = false;
}

}

