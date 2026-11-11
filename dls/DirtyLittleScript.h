//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Dirty Little Script
//-----------------------------------------------------------------------------
#pragma once
#include <iostream>
#include <string>


#include "core/VM.h"
#include "core/Lexer.h"
#include "core/Parser.h"
#include "toolbox/Tools.h"

#ifndef DLS_NO_EXTRA_FUNCTIONS
#include "functions/MathFunctions.h"
#include "functions/StringFunctions.h"
#include "functions/HelpFunctions.h"
#endif

#include <fstream>
#include <istream>



namespace DLS {
    static bool DEBUG_TOKEN = false;
    std::string Run(const std::string& code);
    std::string loadScript(const std::string& scriptPath);
    // void Init( );
    void Init(int argc = 0, char* argv[] = nullptr, Tools::LogCallback customLogHandler = nullptr);
} // namespace DLS

#ifdef IMPLEMENT_DLS
#include "DirtyLittleScript.cpp"
#endif
