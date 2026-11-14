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
    inline bool _init_done_ = false;
    inline bool isInitialized () { return _init_done_;}

    std::string Run(const std::string& code);
    std::string loadScript(const std::string& scriptPath);

    // init with arguments and loghandler
    void Init(int argc, char* argv[], Tools::LogCallback customLogHandler = nullptr);
    // plain init
    void Init( );

    // compile the code
    bool Compile(VM& vm, const std::string& code);
    // run with vm and return Value
    const Value Run(VM& vm, bool doRewind = true);
} // namespace DLS

#ifdef IMPLEMENT_DLS
#include "DirtyLittleScript.cpp"
#endif
