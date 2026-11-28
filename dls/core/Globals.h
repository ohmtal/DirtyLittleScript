//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Dirty Little Script
//-----------------------------------------------------------------------------
#pragma once

#include "toolbox/ArenaAllocator.h"
#include "core/StructArena.h"

namespace DLS {
    inline ArenaAllocator GarbageList;
    inline StructArena StructsList;
} //DLS
