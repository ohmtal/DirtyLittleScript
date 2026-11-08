//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// VM Flat Tape
//-----------------------------------------------------------------------------
#pragma once

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

namespace DLS {

    void PrintRuntimeError(const std::string& message
    , const std::string& p1 = "",const std::string& p2 = "",const std::string& p3 = "");

    struct ArrayEntry {
        std::vector<Value> Elements;
        void setAssigned(bool isAssigned) {
            if (isAssigned) mAssingCount++;
            else {
#ifdef DLS_DEBUG
                assert(mAssingCount > 0 && "Array Assign counter out of bounds!");
#endif
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

        VariableStorage() {
            mGlobalsRegister.reserve(8192);
        }

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
            #ifdef DLS_DEBUG
            assert (index < mGlobalsRegister.size() && "SAVE: GLOBALS REGISTER OUT OF BOUNDS");
            #endif

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
            #ifdef DLS_DEBUG
            assert (index < mGlobalsRegister.size() && "LOAD: GLOBALS REGISTER OUT OF BOUNDS");
            #endif
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
            #ifdef DLS_DEBUG
            assert(masterIndex < mArrays.size() && "Master index array out of range");
            #endif
            return mArrays[masterIndex].Elements;
        }

        Value getArray(size_t varIndex, U32 index) {
            Value& arrValue = getGlobal(varIndex);
            if (!arrValue.isArray()) [[unlikely]] {
                PrintRuntimeError("This is not a array Value!");
                return Value(0.0);
            }
            size_t masterIndex =(size_t)arrValue.getFastLong();

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
                size_t masterIndex =(size_t)arrValue.getFastLong();
                if (index >= mArrays[masterIndex].Elements.size()) {
                    mArrays[masterIndex].Elements.resize(index + 1);
                }
                if (mArrays[masterIndex].Elements[index].isArray()) {
                    size_t oldIndex = (size_t)mArrays[masterIndex].Elements[index].getFastLong();
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
    // ------------------------------------------
    // Slot
    // ------------------------------------------
    // using OpFunc = void(*)(VM&);
    // union Slot {
    //     OpFunc code;
    //     Value value;
    //     size_t jump_offset;
    //     size_t tapeChunk;
    // };

    // union Slot {
    //     uint64_t code;
    //     Value value;
    //     uint64_t jump_offset;
    //     uint64_t tapeChunk;
    // };


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

    struct VM {
        U32 pc = 0;
        std::vector<U32> tape;
        U32 STK = 0;
        std::vector<Value> stack;

        std::vector<LoopContext> mLoopStack;
        VariableStorage Variables;

        void run() ;
        // void run() {
        //     STK = 0;
        //     while (pc < tape.size()) {
        //         tape[pc++].code(*this);
        //     }
        // }

        VM() {
            Globals::CurrentVM = this;
            stack.resize(1024);
        }
        ~VM() {
            Globals::CurrentVM = nullptr;
        }

        // ------------------------------------------
        // Tape functions
        // ------------------------------------------
        void emitCode(const U32 opcode ) { tape.push_back(opcode); }
        U32 emitValue(Value val) {
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

        void emitTapeChunk(const U32 tapeChunk)   { tape.push_back( tapeChunk); }

        void emitJumpOffset(const U32 offset)   { tape.push_back( offset); }
        void patchJumpOffset(const U32 tapePosition, const U32 offset)   { tape[tapePosition] = offset; }

        U32 tell() { return (U32)tape.size(); }

        void rewindTape(U32 slot) {
            while (tape.size() > slot) {
                tape.pop_back();
            }
        }

        U32& peekSlot(U32 reverseOffset) {
            return tape[pc - reverseOffset];
        }

        U32& advanceSlot() {
#ifdef DLS_DEBUG
            assert (pc < tape.size() && "TAPE OVERFLOW");
#endif
            return tape[pc++];
        }

        inline Value advance_value() {
            Value val;
            std::memcpy(&val, &tape[pc], sizeof(Value));
            pc += 4; // 16 bytes
            return val;
        }

        // ------------------------------------------
        // Stack functions
        // ------------------------------------------
        // ---- pop
        FORCE_INLINE Value& POP_STACK() {
            #ifdef DLS_DEBUG
            assert(STK > 0 && "POP_STACK but stack empty!");
            #endif
            return stack[--STK];
        }
        // ---- peek
        FORCE_INLINE Value& PEEK_STACK() {
            #ifdef DLS_DEBUG
            assert(STK > 0 && "POP_STACK but stack empty!");
            #endif
            return stack[STK - 1];
        }
        // ---- push
        FORCE_INLINE void PUSH_STACK(const Value& val) {
            #ifdef DLS_DEBUG
            if (STK >= stack.size()) [[unlikely]] {
                size_t new_size = stack.empty() ? 8 : stack.size() * 2;
                stack.resize(new_size);
                PrintRuntimeError("STACK RESIZE!", std::to_string(new_size));
            }
            #endif
            stack[STK++] = val;
        }

        U32 STACK_SIZE() {
            return STK;
        }

        // ------------------------------------------
        // Run script  functions
        // ------------------------------------------
        bool PlaceTapeAtScriptFunction(Value& var) {
            if (var.type != ValueType::Function ) [[unlikely]] return false;
            if (pc < tape.size() - 1) Variables.mReturnStack.push_back(pc);
            pc = Variables.mFunctions[(size_t)var.getFastLong()];
            if (pc >= tape.size()) [[unlikely]] return false;
            return true;
        }
    };
    // =========================================================================




    #define OP_CODE_LIST \
    X(op_exit,          0, "exit") \
    X(op_pop_stack,     0, "pop the stack") \
    X(op_load_global,    1, "load a global variable") \
    X(op_save_global,    1, "save a global variable") \
    X(op_load_array,     2, "load a array") \
    X(op_save_array,     2, "save a array") \
    X(op_constuct_array,  1, "construct a array") \
    X(op_literal,        1, "push a value") \
    X(op_add,            0, "math +") \
    X(op_inc,            1, "math ++") \
    X(op_dec,            1, "math --") \
    X(op_sub,            0, "math -") \
    X(op_mul,           0, "math *") \
    X(op_div,            0, "math /") \
    X(op_modulo,         0, "math %") \
    X(op_bit_or,         0, "bit |") \
    X(op_bit_xor,        0, "bit ^") \
    X(op_bit_and,        0, "bit &") \
    X(op_bit_shl,        0, "bit <<") \
    X(op_bit_shr,        0, "bit >>") \
    X(op_or,             0, "logical ||") \
    X(op_and,            0, "logical &&") \
    X(op_greater,        0, "compare >") \
    X(op_lower,          0, "compare <") \
    X(op_equal,          0, "compare ==") \
    X(op_not_equal,      0, "compare !=") \
    X(op_lower_equal,      0, "compare <=") \
    X(op_greater_equal,      0, "compare >=") \
    X(op_jump,               1, "jump") \
    X(op_jump_if_true,       1, "jump true") \
    X(op_jump_if_false,      1, "jump false") \
    X(op_jump_if_true_no_pop,      1, "jump false no pop") \
    X(op_jump_if_false_no_pop,      1, "jump false no pop") \
    X(op_cfunc,      2, "call a c registered func") \
    X(op_func_def,      1, "define a lambda") \
    X(op_return,      0, "return")

    #define X(name, args, comment) name,
    enum OpCodes : uint64_t {
        OP_CODE_LIST
    };
    #undef X

// =============================================================================
// =============================================================================
#ifdef IMPLEMENT_DLS

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
#endif // IMPLEMENT_DLS
} //namespace

