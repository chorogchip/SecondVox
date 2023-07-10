#pragma once

#if defined (DEBUG) || defined (_DEBUG)
#define M_DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#if defined max
#undef max
#endif
#if defined min
#undef min
#endif

#include <cassert>
#include <intrin.h>

#define M_VEC_CALL __vectorcall
#define M_FORCE_INLINE __forceinline
#define M_FALL_THROUGH
#define M_UNREACHABLE assert(false)

#define M_STRINGIZE_DETAIL(x) #x
#define M_STRINGIZE(x) M_STRINGIZE_DETAIL(x)
#define M_LOGERROR(msg) do OutputDebugStringA("error logged in file " __FILE__ " line " M_STRINGIZE(__LINE__) ": " msg "\n" ); while (0)
