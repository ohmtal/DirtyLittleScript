//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
#pragma once

#include <cstdint>

#if defined(_MSC_VER)
// Microsoft Visual Studio Compiler
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
// GCC, Clang, and Intel Compiler (on Linux/macOS)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
// Fallback for other compilers
#define FORCE_INLINE inline
#endif

namespace DLS {
    // ------------------------------------------
    // lazy types
    // ------------------------------------------
    typedef uint8_t  U8;
    typedef uint32_t U32;
    typedef int32_t  S32;
    typedef float    F32;
    typedef int64_t  S64;
    typedef double   F64;

} //namespace
