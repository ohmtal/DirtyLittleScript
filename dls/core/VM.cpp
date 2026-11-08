//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// VM Flat Tape
//-----------------------------------------------------------------------------
#ifdef DLS_DEBUG
#include <cassert>
#endif

#include <cstdint>
#include <memory>
#include <vector>
#include <cstring> //mem ...
#include <span>
#include "Types.h"
#include "Value.h"
#include "FunctionMap.h"
#include "Globals.h"

#ifndef IMPLEMENT_DLS
#include "VM.h"
#endif

namespace DLS {

    #define DISPATCH() goto *dispatch_table[vm.tape[vm.pc++]]
    #define DISPATCH_OPCODE(op) goto *dispatch_table[op]

    void PrintRuntimeError(const std::string& message
    , const std::string& p1,const std::string& p2,const std::string& p3) {
        Tools::errorf("[Runtime-ERROR]: %s %s %s %s\n", message.c_str(),p1.c_str(),p2.c_str(),p3.c_str());
    }
    // -----------------------------------------------------------------------------
    void VM::run()  {
        VM& vm = *this; // for conversation
        STK = 0;

        #define X(name, args, comment) &&handle_##name,
        constexpr void* dispatch_table[] = {
            OP_CODE_LIST
        };
        #undef X

        DISPATCH();
        // ------------------------------------------
        // Exit (last for jump offsets)
        // ------------------------------------------
        handle_op_exit: {
            return;
        }
        // ------------------------------------------
        // POP
        // ------------------------------------------
        handle_op_pop_stack: {
            (void) vm.POP_STACK();
            DISPATCH();
        }

        handle_op_pop_stack_count: {
            U32 count = vm.advanceSlot();
            for (int i = 0; i < count; i++) {
                (void) vm.POP_STACK();
            }
            DISPATCH();
        }

        // ------------------------------------------
        // Variables
        // ------------------------------------------
        handle_op_save_global: {
            U32 varIndex = vm.advanceSlot();
            vm.Variables.saveGlobal(varIndex,vm.POP_STACK());
            DISPATCH();
        }

        handle_op_load_global: {
            U32 varIndex = vm.advanceSlot();
            Value& var = vm.Variables.getGlobal(varIndex);
            if (var.type == ValueType::Function) [[unlikely]] {
                vm.PlaceTapeAtScriptFunction(var);
            } else {
                vm.PUSH_STACK( var );
            }
            DISPATCH();
        }

        // ------------------------------------------
        // Array
        // ------------------------------------------
        handle_op_save_array: {

            U32 varIndex = (U32)vm.advanceSlot();
            U32 dimension = (U32)vm.advanceSlot();

            //-------- ... what a mess ;) ...

            if ( dimension > 1 ) {

                Value assignValue = vm.POP_STACK();
                static std::vector<S64> indexStack;
                indexStack.clear();
                Value arrayIndexValue;
                // PrintRuntimeError("FIXME op_save_array dimensions! = ", std::to_string(dimension), "value:", assignValue.toString());

                // --- Build index Stack ----
                for (U32 i = 0; i < dimension; i++) {
                    arrayIndexValue = vm.POP_STACK();
                    if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
                        PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
                        DISPATCH();
                    }
                    indexStack.push_back(arrayIndexValue.getLong());
                }
                std::reverse(indexStack.begin(), indexStack.end());

                // ---- Load initial variable ----
                Value var = vm.Variables.getGlobal(varIndex);

                // --- loop the index values -----
                for (S64 i = 0; i < dimension; i++) {
                    if (!var.isArray()) [[unlikely]]{
                        PrintRuntimeError("Not a Array - Multidimensional Arrays must be initialied! dimension:", std::to_string(i));
                        DISPATCH();
                    }
                    // PrintRuntimeError("FIXME dimensions! index => ", std::to_string(i),"::", std::to_string(indexStack[i]));
                    if (var.getFastLong() >= vm.Variables.mArrays.size()) [[unlikely]] {
                        PrintRuntimeError("Array Index out of bounds!");
                        DISPATCH();
                    }
                    if (i == dimension - 1) {
                        vm.Variables.saveArray(var, indexStack[i], assignValue);
                        DISPATCH();
                    }
                    /*const*/ std::vector<Value>& curArr = vm.Variables.getArrayByMasterIndex(var.getFastLong());
                    if (indexStack[i] >= curArr.size() ) [[unlikely]]{
                        PrintRuntimeError("Array Index out of bounds!");
                        DISPATCH();
                    }
                    var = curArr[indexStack[i]];
                }

                DISPATCH();
            }

            //--------


            Value assignValue = vm.POP_STACK();
            Value arrayIndexValue = vm.POP_STACK();
            if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
                PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
                DISPATCH();
            }

            vm.Variables.saveArray(varIndex,arrayIndexValue.getUInt(), assignValue);
            DISPATCH();
        }
        // --------------------------------------------------
        handle_op_load_array: {
            U32 varIndex = vm.advanceSlot();
            U32 dimension = (U32)vm.advanceSlot();

            //-------- ... what a mess ;) ...
            if ( dimension > 1 ) {
                static std::vector<S64> indexStack;
                indexStack.clear();
                Value arrayIndexValue;
                // PrintRuntimeError("FIXME op_load_array dimensions! = ", std::to_string(dimension));
                // --- Build index Stack ----
                for (U32 i = 0; i < dimension; i++) {
                    arrayIndexValue = vm.POP_STACK();
                    if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
                        PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
                        DISPATCH();
                    }
                    indexStack.push_back(arrayIndexValue.getLong());
                }
                std::reverse(indexStack.begin(), indexStack.end());

                // ---- Load initial variable ----
                Value var = vm.Variables.getGlobal(varIndex);
                // --- loop the index values -----
                for (S64 i = 0; i < dimension; i++) {
                    if (!var.isArray()) [[unlikely]]{
                        PrintRuntimeError("Not a Array!");
                        vm.PUSH_STACK(Value());
                        DISPATCH();
                    }
                    // PrintRuntimeError("FIXME dimensions! index => ", std::to_string(i),"::", std::to_string(indexStack[i]));
                    if (var.getFastLong() >= vm.Variables.mArrays.size()) [[unlikely]] {
                        PrintRuntimeError("Array Index out of bounds!");
                        vm.PUSH_STACK(Value());
                        DISPATCH();
                    }
                    const std::vector<Value>& curArr = vm.Variables.getArrayByMasterIndex(var.getFastLong());
                    if (indexStack[i] >= curArr.size() ) [[unlikely]]{
                        PrintRuntimeError("Array Index out of bounds!");
                        vm.PUSH_STACK(Value());
                        DISPATCH();
                    }
                    var = curArr[indexStack[i]];
                }
                vm.PUSH_STACK(var);
                DISPATCH();
            }
            //--------
            Value arrayIndexValue = vm.POP_STACK();
            if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
                PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
                DISPATCH();
            }
            vm.PUSH_STACK(vm.Variables.getArray(varIndex,arrayIndexValue.getUInt()));
            DISPATCH();
        }

        handle_op_constuct_array: {
            U32 count = vm.advanceSlot();
            static std::vector<Value> arr;
            arr.clear();
            for (S32 i = 0; i < (S32)count; i++) {
                arr.push_back(vm.POP_STACK());
            }
            std::reverse(arr.begin(), arr.end());

            vm.PUSH_STACK(vm.Variables.createArray(arr));
            DISPATCH();
        }
        // ------------------------------------------
        // Literal
        // ------------------------------------------
        handle_op_literal: {
            vm.PUSH_STACK(vm.advance_value());
            DISPATCH();
        }
        // ------------------------------------------
        // Basic Math
        // ------------------------------------------
        handle_op_add: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();

            a = Value(a.getDouble() + b.getDouble());
            DISPATCH();
        }


        handle_op_inc: {
            U32 varIndex = vm.advanceSlot();
            Value& dstValue = vm.Variables.getGlobal(varIndex);
            dstValue = Value(dstValue.getDouble() + 1.0);
            DISPATCH();
        }
        handle_op_dec: {
            U32 varIndex = vm.advanceSlot();
            Value& dstValue = vm.Variables.getGlobal(varIndex);
            dstValue = Value(dstValue.getDouble() - 1.0);
            DISPATCH();
        }

        handle_op_sub: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getDouble() - b.getDouble());
            DISPATCH();
        }

        handle_op_mul: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getDouble() * b.getDouble());
            DISPATCH();
        }

        handle_op_div: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            double right = b.getDouble();
            if (right == 0.0) [[unlikely]]  {
                a = Value(0.0);
                return ;
            }
            a = Value(a.getDouble() / right);
            DISPATCH();
        }

        handle_op_modulo: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            S64 right = b.getLong();
            if (right == 0.0) [[unlikely]]  {
                a = Value(0.0);
                return ;
            }
            a = Value(a.getLong() % right);
            DISPATCH();
        }
        // ------------------------------------------
        // Bit's
        // ------------------------------------------
        handle_op_bit_or: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getUInt() | b.getUInt());
            DISPATCH();
        }
        handle_op_bit_xor: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getUInt() ^ b.getUInt());
            DISPATCH();
        }
        handle_op_bit_and: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getUInt() & b.getUInt());
            DISPATCH();
        }
        handle_op_bit_shl: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getUInt() << b.getUInt());
            DISPATCH();
        }
        handle_op_bit_shr: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getUInt() >> b.getUInt());
            DISPATCH();
        }
        // ------------------------------------------
        // And/OR
        // ------------------------------------------
        handle_op_or: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getBool() || b.getBool());
            DISPATCH();
        }
        handle_op_and: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getBool() && b.getBool());
            DISPATCH();
        }
        // ------------------------------------------
        // Comparison
        // ------------------------------------------
        handle_op_greater: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getDouble() > b.getDouble());
            DISPATCH();
        }
        handle_op_lower: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getDouble() < b.getDouble());
            DISPATCH();
        }
        handle_op_equal: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            if (a.isString() && b.isString()) [[unlikely]] {
                a = Value(a.getString() == b.getString());
                DISPATCH();
            }
            a = Value(a.getDouble() == b.getDouble());
            DISPATCH();
        }
        handle_op_not_equal: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();

            if (a.isString() && b.isString()) [[unlikely]] {
                a = Value(a.getString() != b.getString());
                DISPATCH();
            }
            a = Value(a.getDouble() != b.getDouble());
            DISPATCH();
        }
        handle_op_lower_equal: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getDouble() <= b.getDouble());
            DISPATCH();
        }
        handle_op_greater_equal: {
            Value& b = vm.POP_STACK();
            Value& a = vm.PEEK_STACK();
            a = Value(a.getDouble() >= b.getDouble());
            DISPATCH();
        }


        // ------------------------------------------
        // JUMP
        // ------------------------------------------
        handle_op_jump: {
            U32 target = vm.advanceSlot();
            vm.pc = target;
            DISPATCH();
        }

        handle_op_jump_if_true: {
            Value a = vm.POP_STACK();
            U32 target = vm.advanceSlot();
            if (a.getBool()) {
                vm.pc = target;
            }
            DISPATCH();
        }
        handle_op_jump_if_false: {
            Value a = vm.POP_STACK();
            U32 target = vm.advanceSlot();
            if (!a.getBool()) {
                vm.pc = target;
            }
            DISPATCH();
        }

        handle_op_jump_if_true_no_pop: {
            Value a = vm.PEEK_STACK();
            U32 target = vm.advanceSlot();
            if (a.getBool()) {
                vm.pc = target;
            }
            DISPATCH();
        }
        handle_op_jump_if_false_no_pop: {
            Value a = vm.PEEK_STACK();
            U32 target = vm.advanceSlot();
            if (!a.getBool()) {
                vm.pc = target;
            }
            DISPATCH();
        }
        // ------------------------------------------
        // C-Function call
        // ------------------------------------------
        handle_op_cfunc: {
            // args count
            U32 symid =  vm.advanceSlot();
            U32 numArgs = vm.advanceSlot();

            const FunctionMap::CallBack* cb = FunctionMap::GetCFunction(symid);
            if (!cb) [[unlikely]]{
                for (U32 i = 0; i < numArgs; ++i) {
                    (void)vm.POP_STACK();
                }
                vm.PUSH_STACK(Value(0));
                DISPATCH();
            }

            // Tools::printf("CALL FUNC: %s [args:%f]\n", SymbolTable::getName(symid).c_str(), numArgs);

            if (numArgs <= 12) {
                Value evaluatedArgsStack[12];

                for (S64 i = numArgs ; i > 0  ; i--) {
                    evaluatedArgsStack[i - 1] = vm.POP_STACK();
                }
                Value returnValue = Value(0);
                bool success = (*cb)(std::span<Value>(evaluatedArgsStack, numArgs), returnValue);

                if (!success) [[unlikely]] {
                    Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
                }
                vm.PUSH_STACK(returnValue);
                DISPATCH();
            } else {
                // --- more than 12 parameters fallback ---
                static std::vector<Value> evaluatedArgs;
                evaluatedArgs.clear();
                evaluatedArgs.resize(numArgs);
                for (S64 i = numArgs ; i > 0  ; i--) {
                    evaluatedArgs[i - 1] = vm.POP_STACK();
                }
                Value returnValue = Value(0);
                bool success = (*cb)(evaluatedArgs, returnValue);

                if (!success) [[unlikely]] {
                    Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
                }
                vm.PUSH_STACK(returnValue);
                DISPATCH();
            }
            DISPATCH();
        }

        // ------------------------------------------
        // Script function def
        // ------------------------------------------
        handle_op_func_def: {
            U32 startPosition =  vm.advanceSlot();
            vm.Variables.mFunctions.push_back(startPosition);
            Value subValue = Value((S64)(vm.Variables.mFunctions.size() - 1));
            subValue.type = ValueType::Function;
            vm.PUSH_STACK(subValue);
            DISPATCH();
        }
        handle_op_return: {
            if (vm.Variables.mReturnStack.size() == 0) {
                vm.pc = vm.tape.size() - 1;
                return;
            }
            vm.pc = vm.Variables.mReturnStack.back();
            vm.Variables.mReturnStack.pop_back();
            DISPATCH();
        }
        // ------------------------------------------
    } //run
} //namespace

