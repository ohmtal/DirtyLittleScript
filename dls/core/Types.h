//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
#pragma once

#include <cstdint>

#if defined(_MSC_VER)
// Microsoft Visual Studio
#define FORCE_INLINE __forceinline
#define FORCE_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
// GCC and Clang
#define FORCE_INLINE inline __attribute__((always_inline))
#define FORCE_NOINLINE __attribute__((noinline))
#else
// Fallback
#define FORCE_INLINE inline
#define FORCE_NOINLINE
#endif


namespace DLS {
    // ------------------------------------------
    // lazy types
    // ------------------------------------------
    typedef uint8_t  U8;
    typedef uint16_t U16;
    typedef uint32_t U32;
    typedef int32_t  S32;
    typedef float    F32;
    typedef int64_t  S64;
    typedef double   F64;

} //namespace
