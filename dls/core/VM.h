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
        std::vector<size_t> mArrayTempAssigned;

        std::vector<size_t> mFunctions;
        std::vector<size_t> mReturnStack;

        VariableStorage() {
            mGlobalsRegister.reserve(8192);
        }

        void reset() {
            mReturnStack.clear();
            mFunctions.clear();
            mArrays.clear();
            mArrayTempAssigned.clear();
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
        void ArrayFreeTemp() {
            for (auto idx : mArrayTempAssigned) {
                mArrays[idx].setAssigned(false);
            }
            mArrayTempAssigned.clear();
        }
        // --------------------------------------------------------------------
        Value createArray(const std::vector<Value>& values, bool assignTemp = false) {
            S64 masterIndex = (S64)reserveArray();

            // i need the temp assigned, when it's added as a func param
            if (assignTemp) [[unlikely]]{
                mArrays[(size_t)masterIndex].setAssigned(true);
                mArrayTempAssigned.push_back((size_t)masterIndex);
            }

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

            //double copy! Value finaleValue = (value.type == ValueType::Array) ? copyArray((size_t)value.getFastLong()) : value;

            // changed to saveArray
            bool doSave =  !arrValue.isArray();
            saveArray(arrValue, index, value);
            // saveArray(arrValue, index, finaleValue);
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
    X(op_pop_stack_count, 1, "pop stack with count") \
    X(op_clear_temp_arrays, 0, "clear temp arrays") \
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
} //namespace
#ifdef IMPLEMENT_DLS
#include "VM.cpp"
#endif // IMPLEMENT_DLS

