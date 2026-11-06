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

#ifdef IMPLEMENT_DLS
    const char* tokenTypeToString(TokenType type) {
        switch (type) {
            case TokenType::Identifier:    return "Identifier";
            case TokenType::Number:        return "Number";
            case TokenType::Assign:        return "Assign";
            case TokenType::StringLiteral: return "StringLiteral";
            case TokenType::LParen:        return "LParen (";
            case TokenType::RParen:        return "RParen )";
            case TokenType::LSquare:        return "LSquare [";
            case TokenType::RSquare:        return "RSquare ]";
            case TokenType::LCurly:        return "LCurly {";
            case TokenType::RCurly:        return "RCurly }";

            // math
            case TokenType::Plus:          return "Plus";
            case TokenType::Minus:         return "Minus";
            case TokenType::Mul:           return "Mul";
            case TokenType::Div:           return "Div";
            case TokenType::Modulo:        return "Modulo";

            // inline math
            case TokenType::PlusPlus:      return "PlusPlus";
            case TokenType::MinusMinus:    return "MinusMinus";

            // if ...
            case TokenType::If:            return "If";
            case TokenType::Greater:       return "Greater";
            case TokenType::Less:          return "Less";
            case TokenType::Equal:         return "Equal";
            case TokenType::NotEqual:      return "Not Equal";
            case TokenType::Else:          return "Else";
            case TokenType::ElIf:          return "ElIf";

            // fn functions
            case TokenType::Fn:            return "Fn";
            case TokenType::End:           return "End";

            // for / while iter
            case TokenType::While:         return "While";
            case TokenType::ForRange:      return "ForRange";

            // break, return
            case TokenType::Break:         return "Break";
            case TokenType::Return:        return "Return";
            case TokenType::Continue:      return "Continue";

            case TokenType::Or:            return "OR ||";
            case TokenType::BitOr:         return "Bit |";
            case TokenType::BitXOr:           return "Bit ^";
            case TokenType::BitAnd:         return "Bit &";
            case TokenType::And:            return "AND &&";
            case TokenType::LowerEqual:     return "LowerEqual";
            case TokenType::GreaterEqual:   return "GreaterEqual";
            case TokenType::SHL:            return "Shift Left <<";
            case TokenType::SHR:            return "Shift Right >>";

            case TokenType::Semicolon:      return "Semicolon";
            case TokenType::EOLToken:       return "EOLToken";
            case TokenType::EOFToken:       return "EOFToken";

            default:                        return "UnknownToken";
        }
    }


    bool isMathOperatorType(const Token& op) {
        return op.mType == TokenType::Plus
        || op.mType == TokenType::Minus
        || op.mType == TokenType::Mul
        || op.mType == TokenType::Div
        ;
    }


    bool isMathAssignOperatorType(TokenType tokenType) {
        return false;
        // return tokenType == TokenType::AssignPlus
        // || tokenType == TokenType::AssignMinus
        // || tokenType == TokenType::AssignMul
        // || tokenType == TokenType::AssignDiv
        // ;
    }

    bool isInlineMathType(const TokenType tokenType) {
        return
        tokenType == TokenType::PlusPlus
        || tokenType == TokenType::MinusMinus
        ;
    }

    // punkt vor strich :P orders of ...

    bool isMathType(const Token& op) {
        return
        isMathOperatorType(op)
        || op.mType == TokenType::Greater
        || op.mType == TokenType::Less
        || op.mType == TokenType::Equal
        || op.mType == TokenType::NotEqual
        || op.mType == TokenType::Or
        || op.mType == TokenType::And
        || op.mType == TokenType::LowerEqual
        || op.mType == TokenType::GreaterEqual
        || op.mType == TokenType::BitAnd
        || op.mType == TokenType::BitOr
        || op.mType == TokenType::BitXOr
        || op.mType == TokenType::SHL
        || op.mType == TokenType::SHR
        || op.mType == TokenType::Modulo
        || isInlineMathType(op.mType)
        || isMathAssignOperatorType(op.mType)
        ;
    }



    bool isContinueToken(const Token& op) {
        return op.mType != TokenType::EOFToken
        && op.mType != TokenType::RParen
        && op.mType != TokenType::Semicolon
        && op.mType != TokenType::End
        && op.mType != TokenType::Else
        && op.mType != TokenType::ElIf
        && op.mType != TokenType::RSquare
        && op.mType != TokenType::RCurly
        && op.mType != TokenType::EOLToken
        && !isMathType(op)
        ;
    }

#endif // IMPLEMENT_DLS
}//namespace
