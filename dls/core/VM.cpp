//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// VM Flat Tape
//-----------------------------------------------------------------------------
#ifdef DLS_DEBUG
#include <cassert>
#endif

#include "Marshall.h"

#ifndef IMPLEMENT_DLS
#include "VM.h"
#endif

namespace DLS {


    void PrintRuntimeError(const std::string& message
    , const std::string& p1,const std::string& p2,const std::string& p3) {
        Tools::errorf("[Runtime-ERROR]: %s %s %s %s\n", message.c_str(),p1.c_str(),p2.c_str(),p3.c_str());
    }

    enum class MathOp {
        Add,
        Sub,
        Mul,
        Div
    };

    template<MathOp Op>
    void doMath(Value& dst, const Value& a, const Value& b) {
        double da = a.isDouble() ? a.d : (a.isLong() ? static_cast<double>(a.l) : 0.0);
        double db = b.isDouble() ? b.d : (b.isLong() ? static_cast<double>(b.l) : 0.0);

        double res;
        if constexpr (Op == MathOp::Add)      res = da + db;
        else if constexpr (Op == MathOp::Sub) res = da - db;
        else if constexpr (Op == MathOp::Mul) res = da * db;
        else if constexpr (Op == MathOp::Div) {
            if (db == 0.0) [[unlikely]] { dst = Value(); return; }
            res = da / db;
        }

        dst.d = res;
        dst.type = ValueType::Double;
    }


    // =========================================================================
    // Variable Storage
    // =========================================================================
    bool VariableStorage::getArrayByMasterIndexSafe(size_t masterIndex, ValueVector& result) {
        if (masterIndex >= mArrays.size() ) return false;
        result  = mArrays[masterIndex].Elements;
        return true;
    }

    // void VariableStorage::saveArray(size_t varIndex, U32 index, const Value& value) {
    //     Value arrValue = getGlobal(varIndex);
    //
    //     //double copy! Value finaleValue = (value.type == ValueType::Array) ? copyArray((size_t)value.getFastLong()) : value;
    //
    //     // changed to saveArray
    //     bool doSave =  !arrValue.isArray();
    //     saveArray(arrValue, index, value);
    //     // saveArray(arrValue, index, finaleValue);
    //     if (doSave) saveGlobal(varIndex,arrValue);
    //
    // }
    //
    // void VariableStorage::saveArray(Value& arrValue, U32 index, const Value& value) {
    //
    //     if (index  >= DLS_MAX_ARRAY_ELEMENTS - 1) [[unlikely]]{
    //         PrintRuntimeError("Array index overflow max array elements!");
    //         return;
    //     }
    //
    //     Value finaleValue = (value.type == ValueType::Array) ? copyArray((size_t)value.getFastLong()) : value;
    //
    //     if (arrValue.isArray()) {
    //         size_t masterIndex =(size_t)arrValue.getFastLong();
    //         if (index >= mArrays[masterIndex].Elements.size()) {
    //             mArrays[masterIndex].Elements.resize(index + 1);
    //         }
    //         if (mArrays[masterIndex].Elements[index].isArray()) {
    //             size_t oldIndex = (size_t)mArrays[masterIndex].Elements[index].getFastLong();
    //             mArrays[oldIndex].setAssigned(false);
    //         }
    //         // copyArray already set AssingCount
    //         mArrays[masterIndex].Elements[index] = finaleValue ;
    //     } else {
    //         size_t masterIndex = mArrays.size();
    //         mArrays.push_back({});
    //         arrValue = Value((S64)masterIndex);
    //         arrValue.type = ValueType::Array;
    //         if (index >= mArrays[masterIndex].Elements.size()) {
    //             mArrays[masterIndex].Elements.resize(index + 1);
    //         }
    //         mArrays[masterIndex].Elements[index] = finaleValue ;
    //     }
    // }
    //
    // Value VariableStorage::copyArray(size_t curMasterIndex){
    //     size_t newIndex = reserveArray();
    //     mArrays[newIndex].Elements = mArrays[curMasterIndex].Elements;
    //     mArrays[newIndex].setAssigned(true); //new entry
    //     Value result = Value( (S64) newIndex);
    //     result.type = ValueType::Array;
    //     return result;
    // }
    //
    // size_t VariableStorage::reserveArray(){
    //     for (size_t i = 0; i < mArrays.size(); i++) {
    //         if (!mArrays[i].isAssigned()) return i;
    //     }
    //     size_t result = mArrays.size();
    //     mArrays.push_back({});
    //     return result;
    // }
    //
    // Value VariableStorage::getArray(size_t varIndex, U32 index) {
    //     Value& arrValue = getGlobal(varIndex);
    //     if (!arrValue.isArray()) [[unlikely]] {
    //         PrintRuntimeError("This is not a array Value!");
    //         return Value(0.0);
    //     }
    //     size_t masterIndex =(size_t)arrValue.getFastLong();
    //
    //     if (index >= mArrays[masterIndex].Elements.size()) [[unlikely]] {
    //         // mArrays[masterIndex].Elements.resize(index + 1);
    //         PrintRuntimeError("Array index out of bounds.");
    //         return Value();
    //     }
    //     return mArrays[masterIndex].Elements[index];
    //
    // }
    //
    // ValueVector& VariableStorage::getArrayByMasterIndex(size_t masterIndex){
    //     #ifdef DLS_DEBUG
    //     assert(masterIndex < mArrays.size() && "Master index array out of range");
    //     #endif
    //     return mArrays[masterIndex].Elements;
    // }
    //
    // Value VariableStorage::createArray(const ValueVector& values, bool assignTemp) {
    //     S64 masterIndex = (S64)reserveArray();
    //
    //     // i need the temp assigned, when it's added as a func param
    //     if (assignTemp) [[unlikely]]{
    //         mArrays[(size_t)masterIndex].setAssigned(true);
    //         mArrayTempAssigned.push_back((size_t)masterIndex);
    //     }
    //
    //     Value result = Value(masterIndex);
    //     result.type = ValueType::Array;
    //     mArrays[masterIndex].Elements = values;
    //     return result;
    // }
    //
    void VariableStorage::ArrayFreeTemp(){
        for (auto idx : mArrayTempAssigned) {
            mArrays[idx].setAssigned(false);
        }
        mArrayTempAssigned.clear();
    }

    bool VariableStorage::getGlobalVariableValueRef(const std::string& varName, Value& ret){
        S64 index = getGlobalRegister(SymbolTable::insert(varName));
        if (index < 0 ) return false;
        ret = getGlobal((size_t)index);
        return true;
    }

    // Value& VariableStorage::getGlobal(size_t index){
    //     #ifdef DLS_DEBUG
    //     assert (index < mGlobalsRegister.size() && "LOAD: GLOBALS REGISTER OUT OF BOUNDS");
    //     #endif
    //     return mGlobalsRegister[index];
    // }
    //
    // void VariableStorage::saveGlobal(size_t index, const Value& value) {
    //     #ifdef DLS_DEBUG
    //     assert (index < mGlobalsRegister.size() && "SAVE: GLOBALS REGISTER OUT OF BOUNDS");
    //     #endif
    //
    //     if (mGlobalsRegister[index].isArray()) [[unlikely]] {
    //         size_t oldIndex = mGlobalsRegister[index].getFastLong();
    //         mArrays[oldIndex].setAssigned(false);
    //     }
    //     if (value.type == ValueType::Array) [[unlikely]] {
    //         mGlobalsRegister[index] = copyArray((size_t)value.getFastLong());
    //         return;
    //     }
    //
    //     mGlobalsRegister[index] = value;
    // }

    size_t VariableStorage::reserveGlobal(U32 symId){
        S64 index = getGlobalRegister(symId);
        if (index < 0 ) {
            index = mGlobalsRegister.size();
            mGlobalsLookUp[symId] = index;
            mGlobalsRegister.push_back(Value());
        }
        return (size_t) index;
    }

    S64 VariableStorage::getGlobalRegister(U32 symId){
        auto it = mGlobalsLookUp.find(symId);
        if (it != mGlobalsLookUp.end()) [[likely]] {
            return it->second;
        }
        return -1;
    }

    void VariableStorage::reset(){
        mReturnStack.clear();
        mFunctions.clear();
        mArrays.clear();
        mArrayTempAssigned.clear();
        mGlobalsRegister.clear();
        mGlobalsLookUp.clear();
    }

    VariableStorage::VariableStorage(){
        mGlobalsRegister.reserve(65536);
    }
    // =========================================================================
    // Virtual Machine
    // =========================================================================
    void VM::rewindTape(U32 slot){
        while (tape.size() > slot) {
            tape.pop_back();
        }
    }

    void VM::rewind(){ pc = 0;}

    U32 VM::tell(){ return (U32)tape.size(); }

    void VM::patchJumpOffset(const U32 tapePosition, const U32 offset){ tape[tapePosition] = offset; }

    void VM::emitJumpOffset(const U32 offset){ tape.push_back( offset); }

    void VM::emitTapeChunk(const U32 tapeChunk){ tape.push_back( tapeChunk); }

    U32 VM::emitValue(Value val) {
        #ifdef DLS_DEBUG
        assert(sizeof(Value) <= 4 * sizeof(U32) && "Value size missmatch! Only 16 byte Values allowed.");
        #endif
        U32 startIp = (U32)tape.size();
        tape.resize(startIp + 4);

        if (sizeof(Value) < 16) {
            tape[startIp] = 0;
            tape[startIp + 1] = 0;
            tape[startIp + 2] = 0;
            tape[startIp + 3] = 0;
        }

        std::memcpy(&tape[startIp], &val, sizeof(Value));
        return startIp;
    }

    void VM::emitCode(const U32 opcode){ tape.push_back(opcode); }

    // -----------------------------------------------------------------------------
    #define DISPATCH() goto *dispatch_table[tape[pc++]]
    #define DISPATCH_OPCODE(op) goto *dispatch_table[op]

    // -----------------------------------------------------------------------------
    void VM::run()  {
        if (tape.size() == 0 || pc + 1 >= tape.size()) [[unlikely]]{
            return;
        }

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
            Value& var = Variables.getGlobalRef(varIndex);
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
                    const ValueVector& curArr = Variables.getArrayByMasterIndex(var.getFastLong());
                    if (idx < 0 || idx >= static_cast<S64>(curArr.size())) [[unlikely]] {
                        PrintRuntimeError("Array Index out of bounds: ", std::to_string(idx));
                        DISPATCH();
                    }

                    var = curArr[idx];
                }

                DISPATCH();
            }

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

        // ------------------------------------------
        // Literal
        // ------------------------------------------
        handle_op_literal: {
            PUSH_STACK(advance_value());
            DISPATCH();
        }
        // ------------------------------------------
        // Basic Math
        // ------------------------------------------
        handle_op_add: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            doMath<MathOp::Add>(a,a,b);
            // a = Value(a.getDouble() + b.getDouble());
            DISPATCH();
        }

        handle_op_assign_add: {
            U32 varIndex = advanceSlot();
            Value b = POP_STACK();
            Value& var = Variables.getGlobalRef(varIndex);
            doMath<MathOp::Add>(var,var,b);
            // var = Value( var.getDouble() + b.getDouble() );
            DISPATCH();
        }
        handle_op_assign_sub: {
            U32 varIndex = advanceSlot();
            Value b = POP_STACK();
            Value& var = Variables.getGlobalRef(varIndex);
            doMath<MathOp::Sub>(var,var,b);
            // var = Value( var.getDouble() - b.getDouble() );
            DISPATCH();
        }
        handle_op_assign_mul: {
            U32 varIndex = advanceSlot();
            Value b = POP_STACK();
            Value& var = Variables.getGlobalRef(varIndex);
            doMath<MathOp::Mul>(var,var,b);
            // var = Value( var.getDouble() * b.getDouble() );
            DISPATCH();
        }
        handle_op_assign_div: {
            U32 varIndex = advanceSlot();
            Value b = POP_STACK();
            Value& var = Variables.getGlobalRef(varIndex);
            doMath<MathOp::Div>(var,var,b);
            // double bval = b.getDouble();
            // if (bval == 0.0) [[unlikely]] {
            //     var = Value(0.0);
            // } else {
            //     var = Value( var.getDouble() / b.getDouble() );
            // }
            DISPATCH();
        }

        handle_op_inc: {
            U32 varIndex = advanceSlot();
            Value& dstValue = Variables.getGlobalRef(varIndex);

            if (dstValue.isDouble()) [[likely]]
                dstValue = Value(dstValue.getDouble() + 1.0);
            else
                dstValue = Value(dstValue.getLong() + 1);

            DISPATCH();
        }

        handle_op_dec: {
            U32 varIndex = advanceSlot();
            Value& dstValue = Variables.getGlobalRef(varIndex);
            if (dstValue.isDouble()) [[likely]]
                dstValue = Value(dstValue.getDouble() - 1.0);
            else
                dstValue = Value(dstValue.getLong() - 1);

            DISPATCH();
        }

        handle_op_sub: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            doMath<MathOp::Sub>(a,a,b);
            // a = Value(a.getDouble() - b.getDouble());
            DISPATCH();
        }

        handle_op_mul: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            doMath<MathOp::Mul>(a,a,b);
            // a = Value(a.getDouble() * b.getDouble());
            DISPATCH();
        }

        handle_op_div: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            doMath<MathOp::Div>(a,a,b);
            // double right = b.getDouble();
            // if (right == 0.0) [[unlikely]]  {
                // a = Value(0.0);
                // return ;
            // }
            // a = Value(a.getDouble() / right);
            DISPATCH();
        }

        handle_op_modulo: {
            Value b = POP_STACK();
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
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getUInt() | b.getUInt());
            DISPATCH();
        }
        handle_op_bit_xor: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getUInt() ^ b.getUInt());
            DISPATCH();
        }
        handle_op_bit_and: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getUInt() & b.getUInt());
            DISPATCH();
        }
        handle_op_bit_shl: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getUInt() << b.getUInt());
            DISPATCH();
        }
        handle_op_bit_shr: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getUInt() >> b.getUInt());
            DISPATCH();
        }
        // ------------------------------------------
        // And/OR
        // ------------------------------------------
        handle_op_or: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getBool() || b.getBool());
            DISPATCH();
        }
        handle_op_and: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getBool() && b.getBool());
            DISPATCH();
        }
        // ------------------------------------------
        // Comparison
        // ------------------------------------------
        handle_op_greater: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getDouble() > b.getDouble());
            DISPATCH();
        }
        handle_op_lower: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getDouble() < b.getDouble());
            DISPATCH();
        }
        handle_op_equal: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            if (a.isString() && b.isString()) [[unlikely]] {
                a = Value(a.getString() == b.getString());
                DISPATCH();
            }
            a = Value(a.getDouble() == b.getDouble());
            DISPATCH();
        }
        handle_op_not_equal: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);

            if (a.isString() && b.isString()) [[unlikely]] {
                a = Value(a.getString() != b.getString());
                DISPATCH();
            }
            a = Value(a.getDouble() != b.getDouble());
            DISPATCH();
        }
        handle_op_lower_equal: {
            Value b = POP_STACK();
            Value& a = PEEK_STACK(0);
            a = Value(a.getDouble() <= b.getDouble());
            DISPATCH();
        }
        handle_op_greater_equal: {
            Value b = POP_STACK();
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
                bool success = (*cb)(*this, ValueSpan(evaluatedArgsStack, numArgs), returnValue);

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
                    success = (*cb)(*this, evaluatedArgs, returnValue);
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
        handle_op_load_field: {
            U32 varIndex = advanceSlot();

            U32& fieldSlot = advanceSlotRef();
            U32 fieldSymId = fieldSlot;

            Value result = Value();

            Value& var = Variables.getGlobalRef(varIndex);

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

            U32& fieldSlot = advanceSlotRef();
            U32 fieldSymId = fieldSlot;

            Value src = POP_STACK();
            Value& var = Variables.getGlobalRef(varIndex);

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

        // ------------------------------------------
        // forRange
        // ------------------------------------------
        handle_op_for_range_start: {
            U32 varIndex    = tape[pc];
            U32 exitTarget  = tape[pc+1];

            // -- cast limit to long --
            Value& limitValue = PEEK_STACK(0);
            if (!limitValue.isLong()) limitValue = Value(limitValue.getLong());
            // --- check it's > 0 else exit ---
            if (limitValue.l <= 0) {
                POP_STACK();
                pc = exitTarget;
                DISPATCH();
            }
            // --- we move the tape over varIndex and bodyStartTarget
            pc  += 2;
            // --- set iter variable to 0 and push copy to stack ---
            Value& iterVar = Variables.getGlobalRef(static_cast<size_t>(varIndex));
            iterVar = Value(0);
            PUSH_STACK(iterVar);

            DISPATCH();
        }
        // -----
        handle_op_for_range_jump: {
            U32 varIndex        = tape[pc];
            U32 bodyStartTarget = tape[pc+1];

            Value& iter  = PEEK_STACK(0);
            Value& limit = PEEK_STACK(1);
            iter.l++;

            if ( iter.l >= limit.l ) [[unlikely]] {
                pc  += 2; // move the tape over varIndex and bodyStartTarget
                STK -= 2; // 2 times pop stack
                DISPATCH();
            }
            Value& globVar = Variables.getGlobalRef(static_cast<size_t>(varIndex)) ;
            globVar.type = ValueType::Long;
            globVar.l = iter.l;
            pc = bodyStartTarget;

            DISPATCH();
        }

        // handle_op_for_range_jump: {
        //     U32 varIndex  = tape[pc];
        //     U32 exitTarget    = tape[pc+1];
        //     pc+=2;
        //     Value& dstValue = Variables.getGlobal(static_cast<size_t>(varIndex));
        //     Value& limitValue = PEEK_STACK(0);
        //     if ( dstValue.l + 1 >= limitValue.l ) {
        //         pc = exitTarget;
        //         DISPATCH();
        //     }
        //
        //     if (dstValue.isLong()) [[likely]] {
        //         dstValue.l++;
        //     } else {
        //         dstValue = Value(dstValue.getLong() + 1);
        //     }
        //
        //     DISPATCH();
        // }

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



} //namespace

