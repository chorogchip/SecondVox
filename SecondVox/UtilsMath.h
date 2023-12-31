#pragma once

#include "Macros.h"

#include <bit>

namespace vox::utils
{

constexpr inline float UM_PI = 3.14159265358979f;

template<typename T>
M_FORCE_INLINE constexpr T UMUpperBoundLog2(T x)
{
    return sizeof(T) * static_cast<T>(8) - static_cast<T>(1) - std::countl_zero(x);
}

template<typename T>
M_FORCE_INLINE constexpr T UMPow2(T x)
{
    return static_cast<T>(1) << x;
}

template<typename T>
M_FORCE_INLINE constexpr T UMMakePow2(T x)
{
    return Pow2(UpperBoundLog2(x));
}

}