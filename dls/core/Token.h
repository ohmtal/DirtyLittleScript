//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Token
//-----------------------------------------------------------------------------
#pragma once

#include <string>

namespace DLS{

    enum class TokenType {
        Identifier
        , Number
        , Assign
        , StringLiteral
        , LParen
        , RParen
        , LSquare, RSquare
        , LCurly, RCurly
        , Dot
        // math
        , Plus , Minus , Mul , Div
        , PlusPlus, MinusMinus
        // if ...
        , If,  Else , ElIf
        , Greater, Less, Equal, NotEqual
        // fn functions
        , Fn, End
        // break, return
        , Break, Return
        , Continue
        , Semicolon
        , BackSlash
        , While
        , ForRange
        , Or, And, BitOr, BitAnd
        , BitXOr
        , LowerEqual, GreaterEqual
        , SHL, SHR
        , Modulo
        , NoToken // for peekPrev pos < 1
        , EOLToken
        , EOFToken
    };


    struct Token {
        TokenType mType;
        std::string mValue;
    };

    const char* tokenTypeToString(TokenType type);
    bool isMathOperatorType(const Token& op);
    bool isMathAssignOperatorType(TokenType tokenType);
    bool isInlineMathType(const TokenType tokenType);
    bool isMathType(const Token& op);
    bool isContinueToken(const Token& op);
}//namespace
#ifdef IMPLEMENT_DLS
#include "Token.cpp"
#endif // IMPLEMENT_DLS
