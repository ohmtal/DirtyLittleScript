//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Parser: Pratt (top-down operator precedence parser /Precedence Climbing Parser)
//-----------------------------------------------------------------------------

#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

#include "Token.h"
#include "Lexer.h"
#include "VM.h"
#include "FunctionMap.h"

#ifndef IMPLEMENT_DLS
#include "Parser.h"
#endif

namespace DLS{

static bool ArrayConstructed = false;
static bool ArrayAssign = false;
// -------------------------------------------------------------------------
void Parser::PrintParseError(const std::string& message
, const std::string& p1 ,const std::string& p2 ,const std::string& p3 ) {
    Tools::errorf("[Parser-ERROR LINE:%d]: %s %s %s %s\n",lineNumber, message.c_str(),p1.c_str(),p2.c_str(),p3.c_str());
}
void Parser::PrintParseInfo(const std::string& message
, const std::string& p1 ,const std::string& p2 ,const std::string& p3 ) {
    Tools::printf("[Parser-INFO LINE:%d]: %s %s %s %s\n",lineNumber, message.c_str(),p1.c_str(),p2.c_str(),p3.c_str());
}
// -------------------------------------------------------------------------
int Parser::getPrecedence(TokenType type){
    if (isInlineMathType(type)) return 100;
    if (type == TokenType::Mul || type == TokenType::Div ||
        type == TokenType::Modulo ) return 90;
    if (type == TokenType::Plus || type == TokenType::Minus) return 80;
    if (type == TokenType::SHL || type == TokenType::SHR) return 70;

    if (type == TokenType::Less || type == TokenType::Greater ||
        type == TokenType::LowerEqual || type == TokenType::GreaterEqual) return 60;

    if (type == TokenType::Equal || type == TokenType::NotEqual) return 50;

    if (type == TokenType::BitAnd) return 40;
    if (type == TokenType::BitXOr) return 30;
    if (type == TokenType::BitOr)  return 20;

    if (type == TokenType::And || type == TokenType::Or) return 10;

    if (type == TokenType::Assign) return 0;
     if (isMathAssignOperatorType(type)) return 0;
    return -1;
}
// -------------------------------------------------------------------------
// parsePrimary
// -------------------------------------------------------------------------
bool Parser::parsePrimary() {
    Token t = peek();
    TokenType curType = peek().mType;

    switch (curType) {
        // ~~~~ Number ~~~~
        case TokenType::Number: {
            Token t = advance();
            std::string rawValue = t.mValue;
            Value evaluatedValue;

            char* endptr = nullptr;
            bool  haveDot = rawValue.find('.') != std::string::npos;
            double resDouble = std::strtod(rawValue.c_str(), &endptr);
            if (rawValue.empty() || *endptr != '\0') {
                evaluatedValue = Value(0);
            } else {
                if (haveDot) evaluatedValue = Value(resDouble);
                else evaluatedValue =  Value((S64)resDouble);
            }
            vm->emitCode(op_literal );
            vm->emitValue( evaluatedValue );

        }
        break;

        // ~~~~ String ~~~~
        case TokenType::StringLiteral: {
            Token t = advance();
            vm->emitCode(op_literal );
            vm->emitValue( Value(t.mValue) );
        }
        break;
        // ~~~~ LParen/RParen () ~~~~
        case TokenType::LParen: {
            advance();

            if (!getArg("inside Paren")) return false;

            if (peek().mType == TokenType::RParen) {
                advance(); //  ')'
            } else {
                PrintParseError("Error: Missing closing bracket");
                return false;
            }
            // NOTE nothing else to do here ?!
        }
        break;
        // ~~~~ Identifier ~~~~
        case TokenType::Identifier: {
            advance(); // eat
            U32 symId = SymbolTable::insert(t.mValue);

            if (FunctionMap::IsCFunction(symId)) {
                U32 argCount = 0;
                ArrayConstructed = false;
                ArrayAssign = true;
                while (isContinueToken(peek())) {
                    if (!getArg( "c function parameters" )) return false;
                    argCount++;
                }
                vm->emitCode( op_cfunc );
                vm->emitTapeChunk((size_t)symId);
                vm->emitTapeChunk((size_t)argCount);
                //not faster! vm->emitValue(Value(nullptr));

                if (ArrayConstructed) vm->emitCode(op_clear_temp_arrays);
                ArrayAssign = false;
                ArrayConstructed = false;
                if (requestParam <= 0) vm->emitCode( op_pop_stack );
            }
            else
            if (peek().mType == TokenType::LSquare) {
                U32 dimension = 0;
                do {
                    advance(); //eat LSquare

                    if (!getArg("Square Paren")) return false;

                    if (peek().mType != TokenType::RSquare) {
                        PrintParseError("right square paren missing");
                        return false;
                    }
                    advance(); //RSquare
                    dimension++;
                } while (peek().mType == TokenType::LSquare);


                if (peek().mType == TokenType::Assign) {
                    advance(); //Assign
                    if (!getArg("array assign")) return false;
                    S64 varidx = vm->Variables.getGlobalRegister(symId);
                    if (varidx < 0) {
                        varidx = vm->Variables.reserveGlobal(symId);
                    }
                    vm->emitCode(op_save_array);
                    vm->emitTapeChunk((size_t)varidx);
                    vm->emitTapeChunk((size_t)dimension);
                } else if (S64 varidx = vm->Variables.getGlobalRegister(symId); varidx >= 0)  {
                    vm->emitCode(op_load_array);
                    vm->emitTapeChunk((size_t)varidx);
                    vm->emitTapeChunk((size_t)dimension);

                } else {
                    PrintParseError("Unknown Variable", t.mValue);
                    return false;
                }
            }
            else
            if (peek().mType == TokenType::Assign) {
                advance(); // eat Assign
                if (!getArg("Assign")) return false;
                size_t varidx = vm->Variables.reserveGlobal(symId);
                vm->emitCode(op_save_global);
                vm->emitTapeChunk(varidx);
            }
            else
            if (S64 varidx = vm->Variables.getGlobalRegister(symId); varidx >= 0)  {
                vm->emitCode(op_load_global);
                vm->emitTapeChunk((size_t)varidx);
            }
            else
            if (Value* constansPointer = FunctionMap::getConstants(symId);constansPointer) {
                vm->emitCode(op_literal );
                vm->emitValue(*constansPointer);
            }
            else
            {
                PrintParseError("Unhandled Identifier:", t.mValue);
                return false;
            }
        }
        break;
        // ~~~~ Functions ~~~~
        case TokenType::Fn: if (!parseFn()) return false; break;

        // ~~~~ Curly Array constructor ~~~~
        case TokenType::LCurly: if (!parseCurly()) return false; break;

        // ~~~~ LineBreak ~~~~
        case TokenType::EOLToken:
            lineNumber++;
        case TokenType::Semicolon: {
            advance();
            break;
        }
        // ~~~~ Others ~~~~
        default:
            PrintParseError("parsePrimary :: Unhandled Token:", tokenTypeToString(curType));
            return false;
            break;
    } //switch




    return true;
}

// -------------------------------------------------------------------------
// Math
// -------------------------------------------------------------------------
bool Parser::parseMath(int minPrecedence) {
    if (!parsePrimary()) return false;

    while (isMathType(peek())) {
        Token op = peek();
        int precedence = getPrecedence(op.mType);

        if (precedence < minPrecedence) {
            break;
        }
        advance();

        if (isInlineMathType(op.mType)) {
            size_t pc = vm->tape.size();
            // override op_load_global
            if (pc >= 2 && vm->tape[pc - 2] == op_load_global) {
                if (op.mType == TokenType::PlusPlus) {
                    vm->tape[pc - 2] = op_inc;
                } else { // must be MinusMinus
                    vm->tape[pc - 2] = op_dec;
                }
                // if (requestParam <= 0) vm->emitCode( op_pop_stack );

            } else {
                PrintParseError("Inline Math need Variable!");
                return false;
            }

            continue;
        }
        // ----- we need both ---------
        if (!parseMath(precedence + 1)) return false;
        bool handled = true;
        switch (op.mType) {
            case TokenType::Plus:    vm->emitCode(op_add); break;
            case TokenType::Minus:   vm->emitCode(op_sub); break;
            case TokenType::Mul:     vm->emitCode(op_mul); break;
            case TokenType::Div:     vm->emitCode(op_div); break;
            case TokenType::Modulo:  vm->emitCode(op_modulo); break;
            case TokenType::Greater: vm->emitCode(op_greater); break;
            case TokenType::Less:    vm->emitCode(op_lower); break;
            case TokenType::Equal:   vm->emitCode(op_equal); break;
            case TokenType::NotEqual: vm->emitCode(op_not_equal); break;
            case TokenType::LowerEqual: vm->emitCode(op_lower_equal); break;
            case TokenType::GreaterEqual: vm->emitCode(op_greater_equal); break;
            case TokenType::Or:      vm->emitCode(op_or); break;
            case TokenType::And:     vm->emitCode(op_and); break;
            case TokenType::BitOr:   vm->emitCode(op_bit_or); break;
            case TokenType::BitXOr:  vm->emitCode(op_bit_xor); break;
            case TokenType::BitAnd:  vm->emitCode(op_bit_and); break;
            case TokenType::SHL:     vm->emitCode(op_bit_shl); break;
            case TokenType::SHR:     vm->emitCode(op_bit_shr); break;


            default: handled = false; break;
        }
        // if (handled && requestParam <= 0) vm->emitCode( op_pop_stack );


    } // while
    return true;
}
// -------------------------------------------------------------------------
bool Parser::getArg(const std::string& Where, bool optional ) {
    size_t curPos = tell();
    requestParam++;
    if (!parseMath(0)) return false;
    requestParam--;
    if (tell() == curPos) {
        if (!optional) PrintParseError("Value expected at:", Where);
        return false;
    }
    return true;
}
// -------------------------------------------------------------------------
//  Statements sub: While
// -------------------------------------------------------------------------
bool Parser::parseWhile() {
    advance();
    size_t loop_start_slot = vm->tell();

    if (!getArg("While")) return false;
    vm->emitCode(op_jump_if_false);
    size_t false_jump_slot = vm->tell();
    vm->emitJumpOffset(0); // false placeholder


    // break continue
    LoopContext context;
    context.continue_target = loop_start_slot;
    vm->mLoopStack.push_back(context);


    // main loop:
    while (peek().mType != TokenType::EOFToken &&
        peek().mType != TokenType::End)
    {
        if (!parseStatements()) return false;
    }
    if (peek().mType != TokenType::End) {
        PrintParseError("While missing end!");
        return false;
    }
    advance(); //end

    vm->emitCode(op_jump);
    vm->emitJumpOffset(loop_start_slot);
    vm->patchJumpOffset(false_jump_slot,vm->tell());


    size_t loop_end_slot = vm->tell();

    // handle breaks
    for (size_t break_slot : vm->mLoopStack.back().break_slots) {
        vm->patchJumpOffset(break_slot, loop_end_slot);
    }
    vm->mLoopStack.pop_back();

    return true;
}
// -------------------------------------------------------------------------
//  Statements sub: forRange [VAR] [COUNT]
// -------------------------------------------------------------------------
bool Parser::parseForRange() {
    advance();

    std::string variable = "";
    U32 count = 0;
    if (peek().mType != TokenType::Identifier) {
        PrintParseError("forRange need a variable");
        return false;
    } else {
        variable = peek().mValue;
        advance();
    }
    // if (peek().mType != TokenType::Number) {
    //     PrintParseError("forRange need a count");
    //     return false;
    // } else {
    //     count = (U32)std::abs(atoi(peek().mValue.c_str()));
    //     advance();
    // }

    // ---- emit loop variable and set to -1
    vm->emitCode(op_literal);
    vm->emitValue(Value(-1.0));
    U32 symId = SymbolTable::insert(variable);
    size_t varidx = vm->Variables.reserveGlobal(symId);
    vm->emitCode(op_save_global);
    vm->emitTapeChunk((size_t)varidx);

    // ---- save tape position
    size_t loop_start_slot = vm->tell();

    // ---- increment iter

    vm->emitCode(op_inc);
    vm->emitTapeChunk((size_t)varidx);

    // vm->emitCode(op_load_global);
    // vm->emitTapeChunk((size_t)varidx);
    // vm->emitCode(op_literal);
    // vm->emitValue(Value(1.0));
    // vm->emitCode(op_add);
    // vm->emitCode(op_save_global);
    // vm->emitTapeChunk((size_t)varidx);

    // ---- condition iter < count
    vm->emitCode(op_load_global);
    vm->emitTapeChunk((size_t)varidx);

    if (!getArg("ForRange count")) return false;

    vm->emitCode(op_lower);

    // --- now the same as While.....
    vm->emitCode(op_jump_if_false);
    size_t false_jump_slot = vm->tell();
    vm->emitJumpOffset(0); // false placeholder


    // break continue
    LoopContext context;
    context.continue_target = loop_start_slot;
    vm->mLoopStack.push_back(context);


    // main loop:
    while (peek().mType != TokenType::EOFToken &&
        peek().mType != TokenType::End)
    {
        if (!parseStatements()){
            PrintParseError("ForRange: failed to parse Statements");
            return false;
        }
    }
    if (peek().mType != TokenType::End) {
        PrintParseError("ForRange missing end!");
        return false;
    }
    advance(); //end

    vm->emitCode(op_jump);
    vm->emitJumpOffset(loop_start_slot);
    vm->patchJumpOffset(false_jump_slot,vm->tell());


    size_t loop_end_slot = vm->tell();

    // handle breaks
    for (size_t break_slot : vm->mLoopStack.back().break_slots) {
        vm->patchJumpOffset(break_slot, loop_end_slot);
    }
    vm->mLoopStack.pop_back();

    return true;
}

// -------------------------------------------------------------------------
//  Statements sub: fn
// -------------------------------------------------------------------------
bool Parser::parseFn() {
    advance(); //advanceFN

    // // while (peek().mType == TokenType::EOLToken) advance(); //ear all eol away
    // // if (peek().mType == TokenType::EOFToken) {
    // //     PrintParseError("function def invalid!");
    // //     return false;
    // // }

    size_t rewind_POS = vm->tell();

    // tape should jump over this function when playing
    vm->emitCode(op_func_def);
    size_t func_start_slot = vm->tell();
    vm->emitJumpOffset(0); // start placeholder

    vm->emitCode(op_jump);
    size_t jump_over_slot = vm->tell();
    vm->emitJumpOffset(0); // false placeholder

    vm->patchJumpOffset(func_start_slot, vm->tell());

    // main loop:

    // down want to push it into sub
    S64 saveReq = requestParam;
    requestParam  = 0;
    while (peek().mType != TokenType::EOFToken &&
        peek().mType != TokenType::End)
    {
        if (!parseStatements()) {
            vm->rewindTape(rewind_POS);
            return false;
        }
    }
    requestParam = saveReq; // normalize again
    if (peek().mType != TokenType::End) {
        PrintParseError("Function missing end!");
        vm->rewindTape(rewind_POS);
        return false;
    }
    advance(); //end


    vm->emitCode(op_return);
    vm->patchJumpOffset(jump_over_slot,vm->tell());

    // we supply function id
    if (requestParam <= 0) vm->emitCode( op_pop_stack );

    return true;
}


// -------------------------------------------------------------------------
// Statements sub :  If ElIf Else
// -------------------------------------------------------------------------
bool Parser::parseIfElifElse() {
    advance(); // eat 'if'

    // save jumps for
    std::vector<size_t> end_jump_slots;

    if (!getArg("If condition")) return false;

    vm->emitCode(op_jump_if_false);
    size_t false_jump_slot = vm->tell();
    vm->emitJumpOffset(0); // false placeholder

    // main loop:
    while (peek().mType != TokenType::EOFToken &&
        peek().mType != TokenType::End &&
        peek().mType != TokenType::ElIf &&
        peek().mType != TokenType::Else) {
        if (!parseStatements()) return false;
        }
        vm->emitCode(op_jump);
    end_jump_slots.push_back(vm->tell());
    vm->emitJumpOffset(0); // end placeholder

    // patch end jump
    vm->patchJumpOffset(false_jump_slot, vm->tell());

    // --- ELIF loop ---
    while (peek().mType == TokenType::ElIf) {
        advance(); // eat 'elif'

        if (!getArg("elif condition")) return false;
        vm->emitCode(op_jump_if_false);
        false_jump_slot = vm->tell();
        vm->emitJumpOffset(0); // placeholder nect elif

        while (peek().mType != TokenType::EOFToken &&
            peek().mType != TokenType::End &&
            peek().mType != TokenType::ElIf &&
            peek().mType != TokenType::Else) {
            if (!parseStatements()) return false;;
            }

            // elif jump to end
            vm->emitCode(op_jump);
        end_jump_slots.push_back(vm->tell());
        vm->emitJumpOffset(0);

        // previous false-Jump patch
        vm->patchJumpOffset(false_jump_slot, vm->tell());
    }

    // --- ELSE ---
    if (peek().mType == TokenType::Else) {
        advance(); // eat 'else'

        while (peek().mType != TokenType::EOFToken &&
            peek().mType != TokenType::End) {
            if (!parseStatements()) return false;
            }
    }

    // final end
    if (peek().mType == TokenType::End) {
        advance(); // eat 'end'
    } else {
        PrintParseError("Missing end!");
        return false;
    }

    // --- FINAL JUMPS PATCH ---
    size_t final_destination = vm->tell();
    for (size_t slot : end_jump_slots) {
        vm->patchJumpOffset(slot, final_destination);
    }
    return true;
}
// -------------------------------------------------------------------------
// Statements sub :  curly array constructor
// -------------------------------------------------------------------------
bool Parser::parseCurly() {
    advance();

    size_t count = 0;
    while (peek().mType != TokenType::EOFToken &&
        peek().mType != TokenType::RCurly)
    {
        if (!getArg("Curly Array constuctor")) return false;
        count++;
    }

    if (peek().mType != TokenType::RCurly) {
        PrintParseError("While right curly paren");
        return false;
    }
    advance(); //RCurly

    vm->emitCode(op_constuct_array);
    vm->emitTapeChunk(count);
    vm->emitTapeChunk(ArrayAssign);
    ArrayConstructed = true;
    return true;
}

// -------------------------------------------------------------------------
// Statements
// -------------------------------------------------------------------------
bool Parser::parseStatements() {
    TokenType tokenType = peek().mType;
    switch (tokenType) {
        case TokenType::EOLToken:
            lineNumber++;
        case TokenType::Semicolon: {
            advance();
            break;
        }
        // ----- If / ElIf / Else ----
        case TokenType::If: {
            if (!parseIfElifElse()) return false;
        }
        break;
        case TokenType::While: {
            if (!parseWhile()) return false;
        }
        break;
        case TokenType::ForRange: {
            if (!parseForRange()) return false;
        }
        break;
        case TokenType::Continue: {
            advance(); // eat 'continue'
            if (vm->mLoopStack.empty()) {
                PrintParseError("'continue' found outside of a loop!");
                return false;
            }
            vm->emitCode(op_jump);
            vm->emitJumpOffset(vm->mLoopStack.back().continue_target);
        }
        break;
        case TokenType::Break: {
            advance(); // eat 'break'

            if (vm->mLoopStack.empty()) {
                PrintParseError("'break' found outside of a loop!");
                return false;
            }
            vm->emitCode(op_jump);
            vm->mLoopStack.back().break_slots.push_back(vm->tell());
            vm->emitJumpOffset(0); // placeholder patched in while
        }
        break;

        case TokenType::Return: {
            advance();
            if (isContinueToken(peek())) getArg("Return");
            vm->emitCode(op_return);

        }
        break;


        // ----- default ----
        default: if (!parseMath(0)) return false;; break;
    } //switch

    return true;
}
// -------------------------------------------------------------------------
bool Parser::parse(){
    lineNumber = 1;
    if (mTokens.size() == 0) return false;
    size_t entryTapeSize = vm->tape.size();
    while (peek().mType != TokenType::EOFToken) {
        U32 startPos = tell();
        if (!parseStatements()) {
            vm->rewindTape(entryTapeSize);
            return false;
        }
        if (tell() == startPos) {
            PrintParseError("Failed to parse: reason stuck!");
            vm->rewindTape(entryTapeSize);
            return false;
        }
    }
    vm->emitCode(op_exit);
    return true;
}

} //namespace
