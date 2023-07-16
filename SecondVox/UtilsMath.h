#pragma once

#include "Macros.h"

#include <bit>

namespace vox::utils
{


template<typename T>
M_FORCE_INLINE constexpr T UpperBoundLog2(T x)
{
    return sizeof(T) * static_cast<T>(8) - static_cast<T>(1) - std::countl_zero(x);
}

template<typename T>
M_FORCE_INLINE constexpr T Pow2(T x)
{
    return static_cast<T>(1) << x;
}

template<typename T>
M_FORCE_INLINE constexpr T MakePow2(T x)
{
    return Pow2(UpperBoundLog2(x));
}

}