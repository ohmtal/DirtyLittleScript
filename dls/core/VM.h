//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// VM Flat Tape
//-----------------------------------------------------------------------------
#pragma once


#include <cassert>

#include <cstdint>
#include <memory>
#include <vector>
#include <span>
#include "Types.h"
#include "Value.h"
#include "FunctionMap.h"
#include "Globals.h"

namespace DLS {

    void PrintRuntimeError(const std::string& message
    , const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "");

    struct ArrayEntry {
        std::vector<Value> Elements;
        void setAssigned(bool isAssigned) {
            if (isAssigned) mAssingCount++;
            else {

                assert(mAssingCount > 0 && "Array Assign counter out of bounds!");

                mAssingCount--;
            }
        }
        bool isAssigned() { return mAssingCount > 0;}
    private:
        S64 mAssingCount = 0;
    };
    // ------------------------------------------
    // Variables
    // ------------------------------------------
    struct VariableStorage {

        std::unordered_map<U32,size_t> mGlobalsLookUp;
        std::vector<Value> mGlobalsRegister;

        std::vector<ArrayEntry> mArrays;

        std::vector<size_t> mFunctions;
        std::vector<size_t> mReturnStack;


        void reset() {
            mReturnStack.clear();
            mFunctions.clear();
            mArrays.clear();
            mGlobalsRegister.clear();
            mGlobalsLookUp.clear();
        }


        S64 getGlobalRegister(U32 symId) {
            auto it = mGlobalsLookUp.find(symId);
            if (it != mGlobalsLookUp.end()) [[likely]] {
                return it->second;
            }
            return -1;
        }

        size_t reserveGlobal(U32 symId) {
            S64 index = getGlobalRegister(symId);
            if (index < 0 ) {
                index = mGlobalsRegister.size();
                mGlobalsLookUp[symId] = index;
                mGlobalsRegister.push_back(Value());
            }
            return (size_t) index;
        }

        void saveGlobal(size_t index, const Value& value) {

            assert (index < mGlobalsRegister.size() && "SAVE: GLOBALS REGISTER OUT OF BOUNDS");


            if (mGlobalsRegister[index].isArray()) [[unlikely]] {
                size_t oldIndex = mGlobalsRegister[index].getFastLong();
                mArrays[oldIndex].setAssigned(false);
            }
            if (value.type == ValueType::Array) [[unlikely]] {
                mGlobalsRegister[index] = copyArray((size_t)value.getFastLong());
                return;
            }

            mGlobalsRegister[index] = value;
        }

        Value& getGlobal(size_t index) {

            assert (index < mGlobalsRegister.size() && "LOAD: GLOBALS REGISTER OUT OF BOUNDS");

            return mGlobalsRegister[index];
        }

        bool getGlobalVariableValueRef(const std::string& varName, Value& ret) {
            S64 index = getGlobalRegister(SymbolTable::insert(varName));
            if (index < 0 ) return false;
            ret = getGlobal((size_t)index);
            return true;
        }

        // --------------------------------------------------------------------
        Value createArray(const std::vector<Value>& values) {

            S64 masterIndex = (S64)reserveArray();
            Value result = Value(masterIndex);
            result.type = ValueType::Array;
            mArrays[masterIndex].Elements = values;
            return result;
        }


        /*const*/ std::vector<Value>& getArrayByMasterIndex(size_t masterIndex) {
            assert(masterIndex < mArrays.size() && "Master index array out of range");
            return mArrays[masterIndex].Elements;
        }

        Value getArray(size_t varIndex, U32 index) {
            Value& arrValue = getGlobal(varIndex);
            if (!arrValue.isArray()) [[unlikely]] {
                PrintRuntimeError("This is not a array Value!");
                return Value(0.0);
            }
            size_t masterIndex =arrValue.getUInt();

            if (index >= mArrays[masterIndex].Elements.size()) {
                mArrays[masterIndex].Elements.resize(index + 1);
            }
            return mArrays[masterIndex].Elements[index];

        }

        size_t reserveArray() {
            for (size_t i = 0; i < mArrays.size(); i++) {
                if (!mArrays[i].isAssigned()) return i;
            }
            size_t result = mArrays.size();
            mArrays.push_back({});
            return result;
        }

        Value copyArray(size_t curMasterIndex) {
            size_t newIndex = reserveArray();
            mArrays[newIndex].Elements = mArrays[curMasterIndex].Elements;
            mArrays[newIndex].setAssigned(true); //new entry
            Value result = Value( (S64) newIndex);
            result.type = ValueType::Array;
            return result;
        }

        void saveArray(Value& arrValue , U32 index, const Value& value) {

            Value finaleValue = (value.type == ValueType::Array) ? copyArray((size_t)value.getFastLong()) : value;

            if (arrValue.isArray()) {
                size_t masterIndex =arrValue.getUInt();
                if (index >= mArrays[masterIndex].Elements.size()) {
                    mArrays[masterIndex].Elements.resize(index + 1);
                }
                if (mArrays[masterIndex].Elements[index].isArray()) {
                    size_t oldIndex = mArrays[masterIndex].Elements[index].getFastLong();
                    mArrays[oldIndex].setAssigned(false);
                }
                // copyArray already set AssingCount
                mArrays[masterIndex].Elements[index] = finaleValue ;
            } else {
                size_t masterIndex = mArrays.size();
                mArrays.push_back({});
                arrValue = Value((S64)masterIndex);
                arrValue.type = ValueType::Array;
                if (index >= mArrays[masterIndex].Elements.size()) {
                    mArrays[masterIndex].Elements.resize(index + 1);
                }
                mArrays[masterIndex].Elements[index] = finaleValue ;
            }
        }
        void saveArray(size_t varIndex, U32 index, const Value& value) {
            Value arrValue = getGlobal(varIndex);

            Value finaleValue = (value.type == ValueType::Array) ? copyArray((size_t)value.getFastLong()) : value;

            // changed to saveArray
            bool doSave =  !arrValue.isArray();
            saveArray(arrValue, index, finaleValue);
            if (doSave) saveGlobal(varIndex,arrValue);

        }

    };

    // ------------------------------------------
    // FWD
    // ------------------------------------------
    struct VM;
    using OpFunc = void(*)(VM&);
    // ------------------------------------------
    // Slot
    // ------------------------------------------
    union Slot {
        OpFunc code;
        Value value;
        size_t jump_offset;
        size_t tapeChunk;
    };
    // ------------------------------------------
    // Loops
    // ------------------------------------------
    struct LoopContext {
        size_t continue_target;
        std::vector<size_t> break_slots;
    };

    // ------------------------------------------
    // VM
    // ------------------------------------------
    #define DISPATCH() vm.tape[vm.pc++].code(*vm);

    struct VM {
        size_t pc = 0;
        std::vector<Slot> tape;
        size_t STK = 0;
        std::vector<Value> stack;

        std::vector<LoopContext> mLoopStack;
        VariableStorage Variables;

        void run() {
            while (pc < tape.size()) {
                tape[pc++].code(*this);
            }
        }

        VM() {
            Globals::CurrentVM = this;
            stack.resize(256);
        }
        ~VM() {
            Globals::CurrentVM = nullptr;
        }

        // ------------------------------------------
        // Tape functions
        // ------------------------------------------
        void emitCode(const OpFunc& opcode ) { tape.push_back({.code = opcode}); }
        void emitValue(const Value& value)   { tape.push_back({.value = value}); }
        void emitTapeChunk(const size_t tapeChunk)   { tape.push_back({.tapeChunk = tapeChunk}); }

        void emitJumpOffset(const size_t offset)   { tape.push_back({.jump_offset = offset}); }
        void patchJumpOffset(const size_t tapePosition, const size_t offset)   { tape[tapePosition].jump_offset = offset; }

        size_t tell() { return tape.size(); }

        void rewindTape(size_t slot) {
            while (tape.size() > slot) {
                tape.pop_back();
            }
        }

        Slot& peekSlot(size_t reverseOffset) {
            return tape[pc - reverseOffset];
        }

        Slot& advanceSlot() {

            assert (pc < tape.size() && "TAPE OVERFLOW");

            return tape[pc++];
        }
        // ------------------------------------------
        // Stack functions
        // ------------------------------------------
        Value& POP_STACK() {
            // assert(stack.size() > 0 && "PEEK_STACK but stack empty!");
            // Value result = stack.back(); stack.pop_back();

            assert(STK > 0 && "POP_STACK but stack empty!");

            return stack[--STK];
        }
        Value& PEEK_STACK() {
            // assert(stack.size() > 0 && "PEEK_STACK but stack empty!");
            // Value result = stack.back();

            assert(STK > 0 && "POP_STACK but stack empty!");
            return stack[STK - 1];
        }
        void PUSH_STACK(const Value& val) {
            // stack.push_back(val);
            if (STK + 1 >= stack.size()) [[unlikely]]{
                stack.resize(STK * 2);
            }
            stack[STK++] = val;
        }

        size_t STACK_SIZE() {
            return STK;
        }

        // ------------------------------------------
        // Run script  functions
        // ------------------------------------------
        bool PlaceTapeAtScriptFunction(Value& var) {
            if (var.type != ValueType::Function ) [[unlikely]] return false;
            Variables.mReturnStack.push_back(pc);
            pc = Variables.mFunctions[(size_t)var.getFastLong()];
            return true;
        }
    };
    // =========================================================================
#ifdef IMPLEMENT_DLS
    void PrintRuntimeError(const std::string& message
    , const std::string& p1,const std::string& p2,const std::string& p3) {
        Tools::errorf("[Runtime-ERROR]: %s %s %s %s\n", message.c_str(),p1.c_str(),p2.c_str(),p3.c_str());
    }
    // ------------------------------------------
    // Exit (last for jump offsets)
    // ------------------------------------------
    void op_exit(VM& vm) {}
    // ------------------------------------------
    // POP
    // ------------------------------------------
    inline void op_pop_stack(VM& vm) {
        vm.POP_STACK();
    }
    void op_pop_stack_count(VM& vm) {
        size_t count = vm.advanceSlot().tapeChunk;
        for (size_t i = 0; i < count; i++) {
            vm.POP_STACK();
        }
    }

    // ------------------------------------------
    // Variables
    // ------------------------------------------
    inline void op_save_global(VM& vm) {
        size_t varIndex = vm.advanceSlot().tapeChunk;
        vm.Variables.saveGlobal(varIndex,vm.POP_STACK());
    }

    inline void op_load_global(VM& vm) {
        size_t varIndex = vm.advanceSlot().tapeChunk;
        Value& var = vm.Variables.getGlobal(varIndex);
        if (var.type == ValueType::Function) [[unlikely]] {
            vm.PlaceTapeAtScriptFunction(var);
        } else {
            vm.PUSH_STACK( var );
        }
    }

    // ------------------------------------------
    // Array
    // ------------------------------------------
    void op_save_array(VM& vm) {
        U32 varIndex = (U32)vm.advanceSlot().tapeChunk;
        U32 dimension = (U32)vm.advanceSlot().tapeChunk;

        //-------- ... what a mess ;) ...
        if ( dimension > 1 ) {
            Value assignValue = vm.POP_STACK();
            std::vector<S64> indexStack;
            Value arrayIndexValue;
            // PrintRuntimeError("FIXME op_save_array dimensions! = ", std::to_string(dimension), "value:", assignValue.toString());

            // --- Build index Stack ----
            for (U32 i = 0; i < dimension; i++) {
                arrayIndexValue = vm.POP_STACK();
                if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
                    PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
                    return;
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
                    return;
                }
                // PrintRuntimeError("FIXME dimensions! index => ", std::to_string(i),"::", std::to_string(indexStack[i]));
                if (var.getFastLong() >= vm.Variables.mArrays.size()) [[unlikely]] {
                    PrintRuntimeError("Array Index out of bounds!");
                    return;
                }
                if (i == dimension - 1) {
                   vm.Variables.saveArray(var, indexStack[i], assignValue);
                   return;
                }
                /*const*/ std::vector<Value>& curArr = vm.Variables.getArrayByMasterIndex(var.getFastLong());
                if (indexStack[i] >= curArr.size() ) [[unlikely]]{
                    PrintRuntimeError("Array Index out of bounds!");
                    return;
                }
                var = curArr[indexStack[i]];
            }
            return;
        }
        //--------


        Value assignValue = vm.POP_STACK();
        Value arrayIndexValue = vm.POP_STACK();
        if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
            PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
            return;
        }

        vm.Variables.saveArray(varIndex,arrayIndexValue.getUInt(), assignValue);
    }
    // --------------------------------------------------
    void op_load_array(VM& vm) {
        size_t varIndex = vm.advanceSlot().tapeChunk;
        U32 dimension = (U32)vm.advanceSlot().tapeChunk;

        //-------- ... what a mess ;) ...
        if ( dimension > 1 ) {
            std::vector<S64> indexStack;
            Value arrayIndexValue;
            // PrintRuntimeError("FIXME op_load_array dimensions! = ", std::to_string(dimension));
            // --- Build index Stack ----
            for (U32 i = 0; i < dimension; i++) {
                arrayIndexValue = vm.POP_STACK();
                if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
                    PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
                    return;
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
                    return;
                }
                // PrintRuntimeError("FIXME dimensions! index => ", std::to_string(i),"::", std::to_string(indexStack[i]));
                if (var.getFastLong() >= vm.Variables.mArrays.size()) [[unlikely]] {
                    PrintRuntimeError("Array Index out of bounds!");
                    vm.PUSH_STACK(Value());
                    return;
                }
                const std::vector<Value>& curArr = vm.Variables.getArrayByMasterIndex(var.getFastLong());
                if (indexStack[i] >= curArr.size() ) [[unlikely]]{
                    PrintRuntimeError("Array Index out of bounds!");
                    vm.PUSH_STACK(Value());
                    return;
                }
                var = curArr[indexStack[i]];
            }
            vm.PUSH_STACK(var);
            return;
        }
        //--------
        Value arrayIndexValue = vm.POP_STACK();
        if (!arrayIndexValue.isLong() && !arrayIndexValue.isDouble()) [[unlikely]]{
            PrintRuntimeError("Array index must be an number: ", arrayIndexValue.toString());
            return;
        }
        vm.PUSH_STACK(vm.Variables.getArray(varIndex,arrayIndexValue.getUInt()));
    }

    void op_constuct_array(VM& vm) {
        size_t count = vm.advanceSlot().tapeChunk;
        std::vector<Value> arr;
        for (S32 i = 0; i < (S32)count; i++) {
            arr.push_back(vm.POP_STACK());
        }
        std::reverse(arr.begin(), arr.end());

        vm.PUSH_STACK(vm.Variables.createArray(arr));

        // S64 masterIndex = (S64)vm.Variables.reserveArray();
        // Value result = Value(masterIndex);
        // result.type = ValueType::Array;
        // vm.Variables.mArrays[masterIndex].Elements = arr;
        // vm.PUSH_STACK(result);
    }
    // ------------------------------------------
    // Literal
    // ------------------------------------------
    void op_literal(VM& vm) {
        Value val = vm.tape[vm.pc++].value;
        vm.PUSH_STACK(val);
    }
    // ------------------------------------------
    // Basic Math
    // ------------------------------------------
    void op_add(VM& vm) {
        Value b = vm.POP_STACK();
        Value a = vm.POP_STACK();

        Value res = Value(a.getDouble() + b.getDouble());
        vm.PUSH_STACK(res);
    }

    inline void op_inc(VM& vm) {
        size_t varIndex = vm.advanceSlot().tapeChunk;
        Value& dstValue = vm.Variables.getGlobal(varIndex);
        dstValue = Value(dstValue.getDouble() + 1.0);
        // vm.PUSH_STACK(dstValue);
    }
    void op_dec(VM& vm) {
        size_t varIndex = vm.advanceSlot().tapeChunk;
        Value& dstValue = vm.Variables.getGlobal(varIndex);
        dstValue = Value(dstValue.getDouble() - 1.0);
        // vm.PUSH_STACK(dstValue);
    }


    void op_sub(VM& vm) {
        Value b = vm.POP_STACK();
        Value a = vm.POP_STACK();

        Value res = Value(a.getDouble() - b.getDouble());
        vm.PUSH_STACK(res);
    }

    void op_mul(VM& vm) {
        Value b = vm.POP_STACK();
        Value a = vm.POP_STACK();

        Value res = Value(a.getDouble() * b.getDouble());
        vm.PUSH_STACK(res);
    }

    void op_div(VM& vm) {
        Value b = vm.POP_STACK();
        Value a = vm.POP_STACK();
        double right = b.getDouble();
        if (right == 0.0) [[unlikely]]  {
            vm.PUSH_STACK(Value(0.0));
            return ;
        }
        Value res = Value(a.getDouble() / right);
        vm.PUSH_STACK(res);
    }

    void op_modulo(VM& vm) {
        Value b = vm.POP_STACK();
        Value a = vm.POP_STACK();
        S64 right = b.getLong();
        if (right == 0.0) [[unlikely]]  {
            vm.PUSH_STACK(Value(0.0));
            return ;
        }
        Value res = Value(a.getLong() % right);
        vm.PUSH_STACK(res);
    }
    // ------------------------------------------
    // Bit's
    // ------------------------------------------
    void op_bit_or(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getUInt() | b.getUInt());
        vm.PUSH_STACK(res);
    }
    void op_bit_xor(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getUInt() ^ b.getUInt());
        vm.PUSH_STACK(res);
    }
    void op_bit_and(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getUInt() & b.getUInt());
        vm.PUSH_STACK(res);
    }
    void op_bit_shl(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getUInt() << b.getUInt());
        vm.PUSH_STACK(res);
    }
    void op_bit_shr(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getUInt() >> b.getUInt());
        vm.PUSH_STACK(res);
    }
    // ------------------------------------------
    // And/OR
    // ------------------------------------------
    void op_or(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getBool() || b.getBool());
        vm.PUSH_STACK(res);
    }
    void op_and(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getBool() && b.getBool());
        vm.PUSH_STACK(res);
    }
    // ------------------------------------------
    // Comparison
    // ------------------------------------------
    void op_greater(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() > b.getDouble());
        vm.PUSH_STACK(res);
    }
    void op_lower(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() < b.getDouble());
        vm.PUSH_STACK(res);
    }
    void op_equal(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() == b.getDouble());
        vm.PUSH_STACK(res);
    }
    void op_not_equal(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() != b.getDouble());
        vm.PUSH_STACK(res);
    }
    void op_lower_equal(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() <= b.getDouble());
        vm.PUSH_STACK(res);
    }
    void op_greater_equal(VM& vm) {
        Value b = vm.POP_STACK(); Value a = vm.POP_STACK();
        Value res = Value(a.getDouble() >= b.getDouble());
        vm.PUSH_STACK(res);
    }


    // ------------------------------------------
    // JUMP
    // ------------------------------------------
    void op_jump(VM& vm) {
        size_t target = vm.advanceSlot().jump_offset;
        vm.pc = target;
    }

    void op_jump_if_true(VM& vm) {
        Value a = vm.POP_STACK();
        size_t target = vm.advanceSlot().jump_offset;
        if (a.getBool()) {
            vm.pc = target;
        }
    }
    void op_jump_if_false(VM& vm) {
        Value a = vm.POP_STACK();
        size_t target = vm.advanceSlot().jump_offset;
        if (!a.getBool()) {
            vm.pc = target;
        }
    }

    void op_jump_if_true_no_pop(VM& vm) {
        Value a = vm.PEEK_STACK();
        size_t target = vm.advanceSlot().jump_offset;
        if (a.getBool()) {
            vm.pc = target;
        }
    }
    void op_jump_if_false_no_pop(VM& vm) {
        Value a = vm.PEEK_STACK();
        size_t target = vm.advanceSlot().jump_offset;
        if (!a.getBool()) {
            vm.pc = target;
        }
    }
    // ------------------------------------------
    // C-Function call
    // ------------------------------------------
    void op_cfunc(VM& vm) {
        // args count
        U32 symid = (U32) vm.advanceSlot().tapeChunk;
        U32 numArgs = (U32) vm.advanceSlot().tapeChunk;

        // func cache on tape but not a ms faster?!
        // Value* pointerCacheValue = &vm.advanceSlot().value;
        //
        // FunctionMap::CallBack* cb = nullptr;
        // if ( pointerCacheValue->getPointer()) {
        //     cb = static_cast<FunctionMap::CallBack*>(pointerCacheValue->getPointer());
        // } else {
        //     cb = FunctionMap::GetCFunction(symid);
        //     if (!cb) [[unlikely]]{
        //         for (size_t i = 0; i < numArgs; ++i) {
        //             vm.POP_STACK();
        //         }
        //         //FIXME error!? or special push value
        //         vm.PUSH_STACK(Value(0));
        //         return;
        //     }
        //     *pointerCacheValue = Value(cb);
        // }

        const FunctionMap::CallBack* cb = FunctionMap::GetCFunction(symid);
        if (!cb) [[unlikely]]{
            for (size_t i = 0; i < numArgs; ++i) {
                vm.POP_STACK();
            }
            vm.PUSH_STACK(Value(0));
            return;
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
        } else {
            // --- more than 12 parameters fallback ---
            std::vector<Value> evaluatedArgs;
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
        }
    }

    // ------------------------------------------
    // Script function def
    // ------------------------------------------
    void op_func_def(VM& vm) {
        size_t startPosition =  vm.advanceSlot().jump_offset;
        vm.Variables.mFunctions.push_back(startPosition);
        Value subValue = Value((S64)(vm.Variables.mFunctions.size() - 1));
        subValue.type = ValueType::Function;
        vm.PUSH_STACK(subValue);
    }
    void op_return(VM& vm) {
        if (vm.Variables.mReturnStack.size() == 0) {
            vm.pc = vm.tape.size() - 1;
            return;
        }
        vm.pc = vm.Variables.mReturnStack.back();
        vm.Variables.mReturnStack.pop_back();
    }
    // ------------------------------------------
#endif // IMPLEMENT_DLS
} //namespace

