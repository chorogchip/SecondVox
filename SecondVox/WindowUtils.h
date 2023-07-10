#pragma once

#include <exception>

#include "Macros.h"
#include "WinApiAndDXHeaders.h"


#ifndef ThrowIfFailed
#ifdef M_DEBUG
#define ThrowIfFailed(x) { if (FAILED(x)) { throw std::exception(); } }
#elif
#define ThrowIfFailed(X)
#endif
#endif


namespace vox::wut
{

bool CheckTearingSupport();


}