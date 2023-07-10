#pragma once

#include <immintrin.h>

#include "Macros.h"

namespace vox
{
    using Float128_t = __m128;
    using Sint128_t = __m128i;

    M_FORCE_INLINE constexpr Float128_t operator+(Float128_t a, Float128_t b)
    {
        return _mm_add_ps(a, b);
    }
    M_FORCE_INLINE constexpr Float128_t operator-(Float128_t a, Float128_t b)
    {
        return _mm_sub_ps(a, b);
    }
    M_FORCE_INLINE constexpr Float128_t operator*(Float128_t a, Float128_t b)
    {
        return _mm_mul_ps(a, b);
    }
    M_FORCE_INLINE constexpr Float128_t operator*(Float128_t a, float b)
    {
        return _mm_mul_ps(a, _mm_set_ps1(b));
    }
    M_FORCE_INLINE constexpr Float128_t operator*(float a, Float128_t b)
    {
        return _mm_mul_ps(b, _mm_set_ps1(a));
    }
    M_FORCE_INLINE constexpr Float128_t operator/(Float128_t a, Float128_t b)
    {
        return _mm_div_ps(a, b);
    }
    M_FORCE_INLINE constexpr Float128_t operator/(Float128_t a, float b)
    {
        return _mm_mul_ps(a, _mm_set_ps1(1.0f / b));
    }

    // _mm_dp_ps

    M_FORCE_INLINE constexpr Float128_t FMA(Float128_t a, Float128_t b, Float128_t addend)
    {
        return _mm_fmadd_ps(a, b, addend);
    }



    M_FORCE_INLINE constexpr Sint128_t operator+(Sint128_t a, Sint128_t b)
    {
        return _mm_add_epi32(a, b);
    }

}
