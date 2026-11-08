//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Parser: Pratt (top-down operator precedence parser /Precedence Climbing Parser)
//-----------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

#include "Token.h"
#include "Lexer.h"
#include "VM.h"
#include "FunctionMap.h"

namespace DLS{

class Parser {
private:
    std::vector<Token> mTokens;
    VM* vm;
    size_t mPos = 0;
    S64 requestParam = 0;
    U32 lineNumber = 0;
    // ------------------------------
    Token peek() { return mTokens[mPos]; }
    size_t tell() { return mPos; }
    Token peekNext() { if (mPos + 1 < mTokens.size()) return mTokens[mPos+1]; else return Token(TokenType::EOFToken); }
    Token peekNextNext() { if (mPos + 2 < mTokens.size()) return mTokens[mPos+2]; else return Token(TokenType::EOFToken); }
    Token peekOffset(int32_t offset) {
        int32_t tmpPeek = mPos + offset;
        if (tmpPeek < mTokens.size() && tmpPeek >= 0 )
            return mTokens[tmpPeek]; else return Token(TokenType::EOFToken);
    }
    Token peekPrev() { if (mPos > 1) return mTokens[mPos-1]; else return Token(TokenType::NoToken); }
    Token advance() { if (mPos + 1 < mTokens.size()) return mTokens[mPos++]; else return Token(TokenType::EOFToken);}

    int  getPrecedence(TokenType type);
    bool parsePrimary();
    bool parseMath(int minPrecedence);
    bool getArg(const std::string& Where, bool optional = false);
    bool parseWhile();
    bool parseForRange();
    bool parseFn();
    bool parseIfElifElse();
    bool parseCurly();
    bool parseStatements();

    void PrintParseError(const std::string& message
    , const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "");
    void PrintParseInfo(const std::string& message
    , const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "");
public:
    Parser(std::vector<Token> t, VM* vm) : mTokens(std::move(t)), vm(vm) { }
    bool parse();

};
} //namespace
#ifdef IMPLEMENT_DLS
#include "Parser.cpp"
#endif
