#pragma once
#include <random>
#include <cstdint>
#include <chrono>

#include "Macros.h"

using seed_t = uint64_t;

namespace vox::utils
{


class LinearRandomEngine
{
private:
    seed_t x;

    // Donald Knuth, I trust you
    static constexpr seed_t a = 6364136223846793005ULL;
    static constexpr seed_t c = 1442695040888963407ULL;
public:
    M_FORCE_INLINE LinearRandomEngine(): x(0ULL) {}
    M_FORCE_INLINE LinearRandomEngine(seed_t seed): x(seed) {}
    M_FORCE_INLINE seed_t operator()() {
        x = a * x + c;
        return x;
    }
    M_FORCE_INLINE static constexpr seed_t min() { return (seed_t)0; }
    M_FORCE_INLINE static constexpr seed_t max() { return (seed_t)-1; }
    M_FORCE_INLINE static constexpr seed_t GenerateInstant(seed_t seed)
    {
        return a * seed + c;
    }
};

enum class EnumSeedsForInit : seed_t
{
    ORIGINAL,
    MAP_MANAGER,
    COUNT,
};

M_FORCE_INLINE seed_t GenerateSeedByTime()
{
    return LinearRandomEngine::GenerateInstant(static_cast<seed_t>(
        std::chrono::system_clock::now().time_since_epoch().count()));
}

M_FORCE_INLINE seed_t GenerateSeedBySeed(seed_t seed, EnumSeedsForInit addend)
{
    return LinearRandomEngine::GenerateInstant(seed + (seed_t)addend);
}


}