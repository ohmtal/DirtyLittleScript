//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Math
//-----------------------------------------------------------------------------
#pragma once

namespace DLS {
    void RegisterMathFunctions();
} //namespace
#ifdef IMPLEMENT_DLS
#include "MathFunctions.cpp"
#endif
