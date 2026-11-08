//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Math
//-----------------------------------------------------------------------------
#pragma once
#include <ctime>
#include <cmath>
#include <algorithm>
#include "core/FunctionMap.h"
#include "toolbox/Tools.h"

namespace DLS {
    void RegisterMathFunctions();
} //namespace
#ifdef IMPLEMENT_DLS
#include "MathFunctions.cpp"
#endif
