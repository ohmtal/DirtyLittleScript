//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// VM Flat Tape
//-----------------------------------------------------------------------------
#ifdef DLS_DEBUG
#include <cassert>
#endif

// #include <cstdint>
// #include <memory>
// #include <vector>
// #include <cstring> //mem ...
// #include <span>
// #include "Types.h"
// #include "Value.h"
// #include "FunctionMap.h"
// #include "Globals.h"

#include "Marshall.h"

#ifndef IMPLEMENT_DLS
#include "VM.h"
#endif

namespace DLS {


    void PrintRuntimeError(const std::string& message
    , const std::string& p1,const std::string& p2,const std::string& p3) {
        Tools::errorf("[Runtime-ERROR]: %s %s %s %s\n", message.c_str(),p1.c_str(),p2.c_str(),p3.c_str());
    }

    // -----------------------------------------------------------------------------
    #define DISPATCH() goto *dispatch_table[tape[pc++]]
    #define DISPATCH_OPCODE(op) goto *dispatch_table[op]


    // -----------------------------------------------------------------------------
    void VM::run()  {
        STK = 0;

        ValueVector& globalsRegister = Variables.mGlobalsRegister;

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
            (void) POP_STACK();
            DISPATCH();
        }

        handle_op_pop_stack_count: {
            U32 count = advanceSlot();
            for (int i = 0; i < count; i++) {
                (void) POP_STACK();
            }
            DISPATCH();
        }

        // ------------------------------------------
        // Variables
        // ------------------------------------------
        handle_op_save_global: {
            U32 varIndex = advanceSlot();
            Variables.saveGlobal(varIndex,POP_STACK());
            DISPATCH();
        }

        handle_op_load_global: {
            U32 varIndex = advanceSlot();
            Value& var = Variables.getGlobal(varIndex);
            if (var.type == ValueType::Function) [[unlikely]] {
                PlaceTapeAtScriptFunction(var);
            } else {
                PUSH_STACK( var );
            }
            DISPATCH();
        }

        // ------------------------------------------
        // Array
        // ------------------------------------------
        handle_op_save_array: {

            U32 varIndex = advanceSlot();
            U32 dimension = advanceSlot();

            //-------- ... what a mess ;) ...
            if (dimension > 1) {
                Value assignValue = PEEK_STACK(0);
                Value var = Variables.getGlobal(varIndex);
                for (U32 i = 0; i < dimension; i++) {
                    if (!var.isArray()) [[unlikely]] {
                        PrintRuntimeError("Not an Array - Multidimensional Arrays must be initialized! dimension: ", std::to_string(i));
                        DISPATCH();
                    }

                    if (var.getFastLong() >= Variables.mArrays.size()) [[unlikely]] {
                        PrintRuntimeError("Array Reference out of bounds!");
                        DISPATCH();
                    }
                    U32 stackOffset = 1 + (dimension - 1 - i);
                    Value arrayIndexValue = PEEK_STACK(stackOffset);

                    if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]] {
                        PrintRuntimeError("Array index must be a number: ", arrayIndexValue.toString());
                        DISPATCH();
                    }

                    S64 idx = arrayIndexValue.getLong();
                    if (i == dimension - 1) {
                        Variables.saveArray(var, idx, assignValue);
                        DROP_STACK(dimension + 1);
                        DISPATCH();
                    }
                    ValueVector& curArr = Variables.getArrayByMasterIndex(var.getFastLong());
                    if (idx < 0 || idx >= static_cast<S64>(curArr.size())) [[unlikely]] {
                        PrintRuntimeError("Array Index out of bounds: ", std::to_string(idx));
                        DISPATCH();
                    }

                    var = curArr[idx];
                }

                DISPATCH();
            }

            // if ( dimension > 1 ) {
            //
            //     Value assignValue = POP_STACK();
            //     static std::vector<S64> indexStack;
            //     indexStack.clear();
            //     Value arrayIndexValue;
            //     // PrintRuntimeError("FIXME op_save_array dimensions! = ", std::to_string(dimension), "value:", assignValue.toString());
            //
            //     // --- Build index Stack ----
            //     for (U32 i = 0; i < dimension; i++) {
            //         arrayIndexValue = POP_STACK();
            //         if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
            //             PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
            //             DISPATCH();
            //         }
            //         indexStack.push_back(arrayIndexValue.getLong());
            //     }
            //     std::reverse(indexStack.begin(), indexStack.end());
            //
            //     // ---- Load initial variable ----
            //     Value var = Variables.getGlobal(varIndex);
            //
            //     // --- loop the index values -----
            //     for (S64 i = 0; i < dimension; i++) {
            //         if (!var.isArray()) [[unlikely]]{
            //             PrintRuntimeError("Not a Array - Multidimensional Arrays must be initialied! dimension:", std::to_string(i));
            //             DISPATCH();
            //         }
            //         // PrintRuntimeError("FIXME dimensions! index => ", std::to_string(i),"::", std::to_string(indexStack[i]));
            //         if (var.getFastLong() >= Variables.mArrays.size()) [[unlikely]] {
            //             PrintRuntimeError("Array Index out of bounds!");
            //             DISPATCH();
            //         }
            //         if (i == dimension - 1) {
            //             Variables.saveArray(var, indexStack[i], assignValue);
            //             DISPATCH();
            //         }
            //         /*const*/ ValueVector& curArr = Variables.getArrayByMasterIndex(var.getFastLong());
            //         if (indexStack[i] >= curArr.size() ) [[unlikely]]{
            //             PrintRuntimeError("Array Index out of bounds!");
            //             DISPATCH();
            //         }
            //         var = curArr[indexStack[i]];
            //     }
            //
            //     DISPATCH();
            // }

            //--------


            Value assignValue = POP_STACK();
            Value arrayIndexValue = POP_STACK();
            if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
                PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
                DISPATCH();
            }

            Variables.saveArray(varIndex,arrayIndexValue.getUInt(), assignValue);
            DISPATCH();
        }
        // --------------------------------------------------
        handle_op_load_array: {
            U32 varIndex = advanceSlot();
            U32 dimension = (U32)advanceSlot();

            //-------- ... what a mess ;) ...
            //-------- Viel sauberer und schneller ;) ...
            if (dimension > 1) {
                Value var = Variables.getGlobal(varIndex);
                for (U32 i = 0; i < dimension; i++) {
                    if (!var.isArray()) [[unlikely]] {
                        PrintRuntimeError("Not an Array!");
                        PUSH_STACK(Value());
                        DISPATCH();
                    }

                    if (var.getFastLong() >= Variables.mArrays.size()) [[unlikely]] {
                        PrintRuntimeError("Array Reference out of bounds!");
                        PUSH_STACK(Value());
                        DISPATCH();
                    }

                    U32 stackOffset = dimension - 1 - i;
                    Value arrayIndexValue = PEEK_STACK(stackOffset);

                    if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]] {
                        PrintRuntimeError("Array index must be a number: ", arrayIndexValue.toString());
                        DISPATCH();
                    }

                    S64 idx = arrayIndexValue.getLong();
                    const ValueVector& curArr = Variables.getArrayByMasterIndex(var.getFastLong());

                    if (idx < 0 || idx >= static_cast<S64>(curArr.size())) [[unlikely]] {
                        PrintRuntimeError("Array Index out of bounds: ", std::to_string(idx));
                        PUSH_STACK(Value());
                        DISPATCH();
                    }

                    var = curArr[idx];
                }

                DROP_STACK(dimension);
                PUSH_STACK(var);
                DISPATCH();
            }

            // if ( dimension > 1 ) {
            //     static std::vector<S64> indexStack;
            //     indexStack.clear();
            //     Value arrayIndexValue;
            //     // PrintRuntimeError("FIXME op_load_array dimensions! = ", std::to_string(dimension));
            //     // --- Build index Stack ----
            //     for (U32 i = 0; i < dimension; i++) {
            //         arrayIndexValue = POP_STACK();
            //         if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
            //             PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
            //             DISPATCH();
            //         }
            //         indexStack.push_back(arrayIndexValue.getLong());
            //     }
            //     std::reverse(indexStack.begin(), indexStack.end());
            //
            //     // ---- Load initial variable ----
            //     Value var = Variables.getGlobal(varIndex);
            //     // --- loop the index values -----
            //     for (S64 i = 0; i < dimension; i++) {
            //         if (!var.isArray()) [[unlikely]]{
            //             PrintRuntimeError("Not a Array!");
            //             PUSH_STACK(Value());
            //             DISPATCH();
            //         }
            //         // PrintRuntimeError("FIXME dimensions! index => ", std::to_string(i),"::", std::to_string(indexStack[i]));
            //         if (var.getFastLong() >= Variables.mArrays.size()) [[unlikely]] {
            //             PrintRuntimeError("Array Index out of bounds!");
            //             PUSH_STACK(Value());
            //             DISPATCH();
            //         }
            //         const ValueVector& curArr = Variables.getArrayByMasterIndex(var.getFastLong());
            //         if (indexStack[i] >= curArr.size() ) [[unlikely]]{
            //             PrintRuntimeError("Array Index out of bounds!");
            //             PUSH_STACK(Value());
            //             DISPATCH();
            //         }
            //         var = curArr[indexStack[i]];
            //     }
            //     PUSH_STACK(var);
            //     DISPATCH();
            // }
            //--------
            Value arrayIndexValue = POP_STACK();
            if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
                PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
                DISPATCH();
            }
            PUSH_STACK(Variables.getArray(varIndex,arrayIndexValue.getUInt()));
            DISPATCH();
        }

        handle_op_clear_temp_arrays:{
            Variables.ArrayFreeTemp();
            DISPATCH();
        }

        handle_op_constuct_array: {
            U32 count = advanceSlot();
            U32 assignTemp = advanceSlot();

            {
                ValueVector arr(count);
                for (S32 i = 0; i < (S32)count; i++) {
                    arr[count - 1 - i] = POP_STACK();
                }
                PUSH_STACK(Variables.createArray(arr, assignTemp));

            }
            DISPATCH();
        }

        // handle_op_constuct_array: {
        //     U32 count = advanceSlot();
        //     U32 assignTemp = advanceSlot();
        //     static ValueVector arr;
        //     arr.clear();
        //     for (S32 i = 0; i < (S32)count; i++) {
        //         arr.push_back(POP_STACK());
        //     }
        //     std::reverse(arr.begin(), arr.end());
        //
        //     PUSH_STACK(Variables.createArray(arr, assignTemp));
        //     DISPATCH();
        // }
        // ------------------------------------------
        // Literal
        // ------------------------------------------
        handle_op_literal: {
            // U32 idx = advanceSlot();
            // PUSH_STACK(mLiterals[idx]);
            PUSH_STACK(advance_value());
            DISPATCH();
        }
        // ------------------------------------------
        // Basic Math
        // ------------------------------------------
        handle_op_add: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);

            a = Value(a.getDouble() + b.getDouble());
            DISPATCH();
        }


        handle_op_inc: {
            U32 varIndex = advanceSlot();
            Value& dstValue = Variables.getGlobal(varIndex);
            dstValue = Value(dstValue.getDouble() + 1.0);
            DISPATCH();
        }
        handle_op_dec: {
            U32 varIndex = advanceSlot();
            Value& dstValue = Variables.getGlobal(varIndex);
            dstValue = Value(dstValue.getDouble() - 1.0);
            DISPATCH();
        }

        handle_op_sub: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getDouble() - b.getDouble());
            DISPATCH();
        }

        handle_op_mul: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getDouble() * b.getDouble());
            DISPATCH();
        }

        handle_op_div: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            double right = b.getDouble();
            if (right == 0.0) [[unlikely]]  {
                a = Value(0.0);
                return ;
            }
            a = Value(a.getDouble() / right);
            DISPATCH();
        }

        handle_op_modulo: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
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
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getUInt() | b.getUInt());
            DISPATCH();
        }
        handle_op_bit_xor: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getUInt() ^ b.getUInt());
            DISPATCH();
        }
        handle_op_bit_and: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getUInt() & b.getUInt());
            DISPATCH();
        }
        handle_op_bit_shl: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getUInt() << b.getUInt());
            DISPATCH();
        }
        handle_op_bit_shr: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getUInt() >> b.getUInt());
            DISPATCH();
        }
        // ------------------------------------------
        // And/OR
        // ------------------------------------------
        handle_op_or: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getBool() || b.getBool());
            DISPATCH();
        }
        handle_op_and: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getBool() && b.getBool());
            DISPATCH();
        }
        // ------------------------------------------
        // Comparison
        // ------------------------------------------
        handle_op_greater: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getDouble() > b.getDouble());
            DISPATCH();
        }
        handle_op_lower: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getDouble() < b.getDouble());
            DISPATCH();
        }
        handle_op_equal: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            if (a.isString() && b.isString()) [[unlikely]] {
                a = Value(a.getString() == b.getString());
                DISPATCH();
            }
            a = Value(a.getDouble() == b.getDouble());
            DISPATCH();
        }
        handle_op_not_equal: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);

            if (a.isString() && b.isString()) [[unlikely]] {
                a = Value(a.getString() != b.getString());
                DISPATCH();
            }
            a = Value(a.getDouble() != b.getDouble());
            DISPATCH();
        }
        handle_op_lower_equal: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getDouble() <= b.getDouble());
            DISPATCH();
        }
        handle_op_greater_equal: {
            Value& b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getDouble() >= b.getDouble());
            DISPATCH();
        }


        // ------------------------------------------
        // JUMP
        // ------------------------------------------
        handle_op_jump: {
            U32 target = advanceSlot();
            pc = target;
            DISPATCH();
        }

        handle_op_jump_if_true: {
            Value a = POP_STACK();
            U32 target = advanceSlot();
            if (a.getBool()) {
                pc = target;
            }
            DISPATCH();
        }
        handle_op_jump_if_false: {
            Value a = POP_STACK();
            U32 target = advanceSlot();
            if (!a.getBool()) {
                pc = target;
            }
            DISPATCH();
        }

        handle_op_jump_if_true_no_pop: {
            Value a = PEEK_STACK(0);
            U32 target = advanceSlot();
            if (a.getBool()) {
                pc = target;
            }
            DISPATCH();
        }
        handle_op_jump_if_false_no_pop: {
            Value a = PEEK_STACK(0);
            U32 target = advanceSlot();
            if (!a.getBool()) {
                pc = target;
            }
            DISPATCH();
        }
        // ------------------------------------------
        // C-Function call
        // ------------------------------------------
        handle_op_cfunc: {
            // args count
            U32 symid =  advanceSlot();
            U32 numArgs = advanceSlot();

            const FunctionMap::CallBack* cb = FunctionMap::GetCFunction(symid);
            if (!cb) [[unlikely]]{
                for (U32 i = 0; i < numArgs; ++i) {
                    (void)POP_STACK();
                }
                PUSH_STACK(Value(0));
                DISPATCH();
            }

            // Tools::printf("CALL FUNC: %s [args:%f]\n", SymbolTable::getName(symid).c_str(), numArgs);

            if (numArgs <= 12) {
                Value evaluatedArgsStack[12];

                for (S64 i = numArgs ; i > 0  ; i--) {
                    evaluatedArgsStack[i - 1] = POP_STACK();
                }
                Value returnValue = Value(0);
                bool success = (*cb)(ValueSpan(evaluatedArgsStack, numArgs), returnValue);

                if (!success) [[unlikely]] {
                    Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
                }
                PUSH_STACK(returnValue);

                DISPATCH();
            } else {
                // --- more than 12 parameters fallback ---
                bool success = false;
                Value returnValue = Value(0);
                {

                    ValueVector evaluatedArgs;
                    evaluatedArgs.clear();
                    evaluatedArgs.resize(numArgs);
                    for (S64 i = numArgs ; i > 0  ; i--) {
                        evaluatedArgs[i - 1] = POP_STACK();
                    }
                    success = (*cb)(evaluatedArgs, returnValue);
                }


                if (!success) [[unlikely]] {
                    Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
                }
                PUSH_STACK(returnValue);
                DISPATCH();
            }
            DISPATCH();
        }


        // ------------------------------------------
        // Object Fields
        // ------------------------------------------
        // handle_op_load_field: {
        //     U32 varIndex = advanceSlot();
        //     U32 fieldSymId = advanceSlot();
        //
        //     Value result = Value();
        //
        //     Value& var = Variables.getGlobal(varIndex);
        //
        //     Marshall::Translator* tl = Marshall::getTranslator(var);
        //     if (!tl) [[unlikely]] {
        //         PrintRuntimeError("Failed to get Translator for variable");
        //         PUSH_STACK(result);
        //         DISPATCH();
        //     }
        //     size_t registerIndex;
        //     if (!tl->getFieldRegister(fieldSymId,registerIndex)) [[unlikely]] {
        //         PrintRuntimeError("Failed to get field - unknown fieldname");
        //     } else {
        //         tl->loadValueByRegister(var,registerIndex,result);
        //     }
        //
        //     PUSH_STACK(result);
        //
        //     DISPATCH();
        // }


        handle_op_load_field: {
            U32 varIndex = advanceSlot();

            U32& fieldSlot = advanceSlot();
            U32 fieldSymId = fieldSlot;

            Value result = Value();

            Value& var = Variables.getGlobal(varIndex);

            Marshall::IStructTranslator* tl = Marshall::getStructTranslator(var);
            if (!tl) [[unlikely]] {
                PrintRuntimeError("Failed to get Translator for variable");
                PUSH_STACK(result);
                DISPATCH();
            }


            size_t registerIndex;
            // inline cache :D per bit 31
            if (fieldSymId & 0x80000000) {
                registerIndex = fieldSymId & 0x7FFFFFFF;
            } else {
                if (!tl->getFieldRegister(fieldSymId, registerIndex)) [[unlikely]] {
                    PrintRuntimeError("Failed to set field - unknown fieldname");
                    DISPATCH();
                }
                fieldSlot = registerIndex | 0x80000000;
            }
            tl->loadValueByRegister(var,registerIndex,result);
            PUSH_STACK(result);
            DISPATCH();
        }

        // --- save field ----
        handle_op_save_field: {
            U32 varIndex = advanceSlot();

            U32& fieldSlot = advanceSlot();
            U32 fieldSymId = fieldSlot;

            Value& src = POP_STACK();
            Value& var = Variables.getGlobal(varIndex);

            Marshall::IStructTranslator* tl = Marshall::getStructTranslator(var);
            if (!tl) [[unlikely]] {
                PrintRuntimeError("Failed to get Translator for variable");
                DISPATCH();
            }

            size_t registerIndex;

            if (fieldSymId & 0x80000000) {
                registerIndex = fieldSymId & 0x7FFFFFFF;
            } else {
                if (!tl->getFieldRegister(fieldSymId, registerIndex)) [[unlikely]] {
                    PrintRuntimeError("Failed to set field - unknown fieldname");
                    DISPATCH();
                }
                fieldSlot = registerIndex | 0x80000000;
            }
            tl->saveValueByRegister(var, registerIndex, src);

            DISPATCH();
        }

        // handle_op_save_field: {
        //     U32 varIndex = advanceSlot();
        //     U32 fieldSymId = advanceSlot();
        //     Value& src = POP_STACK();
        //
        //     Value& var = Variables.getGlobal(varIndex);
        //
        //
        //     Marshall::Translator* tl = Marshall::getTranslator(var);
        //     if (!tl) [[unlikely]] {
        //         PrintRuntimeError("Failed to get Translator for variable");
        //         DISPATCH();
        //     }
        //
        //     size_t registerIndex;
        //     if (!tl->getFieldRegister(fieldSymId,registerIndex)) [[unlikely]] {
        //         PrintRuntimeError("Failed to set field - unknown fieldname");
        //     } else {
        //         tl->saveValueByRegister(var,registerIndex,src);
        //     }
        //
        //
        //     DISPATCH();
        // }


        // ------------------------------------------
        // Special forRange :D
        // ------------------------------------------
        handle_op_cast_long: {
            PEEK_STACK(0) = Value(PEEK_STACK(0).getLong());
            DISPATCH();
        }
        handle_op_cast_double: {
            PEEK_STACK(0) = Value(PEEK_STACK(0).getDouble());
            DISPATCH();
        }

        // // as double counter
        // handle_op_for_range_jump: {
        //     U32 varIndex = advanceSlot();
        //     Value& dstValue = Variables.getGlobal(varIndex);
        //     if (dstValue.isDouble()) [[likely]] {
        //         dstValue.d++;
        //     } else {
        //         dstValue = Value(dstValue.getDouble() + 1.0);
        //     }
        //
        //     Value& limitValue = PEEK_STACK(0);
        //     U32 target = advanceSlot();
        //     if (dstValue.l >= limitValue.l) {
        //         (void) POP_STACK();
        //         pc = target;
        //     }
        //     DISPATCH();
        // }

        // as int counter
        handle_op_for_range_jump: {
            U32 varIndex = advanceSlot();
            Value& dstValue = Variables.getGlobal(static_cast<size_t>(varIndex));
            if (dstValue.isLong()) [[likely]] {
                dstValue.l++;
            } else {
                dstValue = Value(dstValue.getLong() + 1);
            }

            Value& limitValue = PEEK_STACK(0);
            U32 target = advanceSlot();
            if (dstValue.l >= limitValue.l) {
                pc = target;
            }
            DISPATCH();
        }

        // ------------------------------------------
        // Script function def
        // ------------------------------------------
        handle_op_func_def: {
            U32 startPosition =  advanceSlot();
            Variables.mFunctions.push_back(startPosition);
            Value subValue = Value((S64)(Variables.mFunctions.size() - 1));
            subValue.type = ValueType::Function;
            PUSH_STACK(subValue);
            DISPATCH();
        }
        handle_op_return: {
            if (Variables.mReturnStack.size() == 0) {
                pc = tape.size() - 1;
                return;
            }
            pc = Variables.mReturnStack.back();
            Variables.mReturnStack.pop_back();

            // 0.62 make sure something is on the stack
            if (STACK_SIZE() == 0 ) {
                PUSH_STACK(Value(0));
            }

            DISPATCH();
        }
        // ------------------------------------------
    } //run

    // =========================================================================
    // Variable Storage
    // =========================================================================
    bool VariableStorage::getArrayByMasterIndexSafe(size_t masterIndex, ValueVector& result) {
        if (masterIndex >= mArrays.size() ) return false;
        result  = mArrays[masterIndex].Elements;
        return true;
    }


} //namespace

