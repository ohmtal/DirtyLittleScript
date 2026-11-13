//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Parts of formatString are borrowed from ElfScript/TorqueScript:
//      Copyright (c) 2012 GarageGames, LLC
//      Copyright (c) 2026 Thomas Hühn (XXTH)
//      SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// String Functions and Object
//-----------------------------------------------------------------------------
#pragma once
#include <string.h>
#include <span>
#include <algorithm>
#include "core/Value.h"
#include "core/FunctionMap.h"

namespace DLS {

    void RegisterStringFunctions( );
    std::string formatString(ValueSpan args);


    namespace StrTools {
        std::string toLower(std::string s);
        std::string addTrailingSlash(std::string path);
        std::string sanitizeFilenameWithUnderScores(std::string name);
        std::string extractFileExt(std::string fileName, bool lowerCase);
        std::string_view extractFilename(std::string_view path);
        std::vector< std::string_view > Tokenize(std::string_view str, const char delimiter);
        std::string getWord(std::string_view str, int no, const char delimiter);
        std::string removeWord(std::string text, int index, const char delimiter);
        std::string setWord(std::string text, int index, std::string_view replace, const char delimiter);
        std::string getWords(std::string_view text, int index, int endIndex, const char delimiter);
        int getWordCount(std::string_view str, const char delimiter);
        std::string toUpper(std::string s);
        void toLowerInline(std::string& s);
        void toUpperInline(std::string& s);
        std::string removePart(std::string s, const std::string& part);
        void rtrim(std::string& s);
        void trim(std::string& s);
        std::string truncate(std::string str, size_t width, bool addPoints);
        std::string trim(std::string_view str, bool bothEnds);
        std::string toLower(std::string s);


    }

} //namespace

#ifdef IMPLEMENT_DLS
#include "StringFunctions.cpp"
#endif
