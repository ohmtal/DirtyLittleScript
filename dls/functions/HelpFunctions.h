//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Help
//-----------------------------------------------------------------------------
#pragma once
#include <ctime>
#include <cmath>
#include <algorithm>
#include "core/FunctionMap.h"
#include "toolbox/Tools.h"

namespace DLS {
    void RegisterHelpFunctions();
} //namespace
#ifdef IMPLEMENT_DLS
#include "HelpFunctions.cpp"
#endif
