#pragma once

#include <iostream>

#include "Macros.h"

namespace vox::utils
{

static inline auto STR_RB = "rb";
static inline auto STR_WB = "wb";

template<typename T>
M_FORCE_INLINE void WriteData(FILE* fp, const T& t)
{
    fwrite(&t, sizeof(t), 1, fp);
}

}