//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Value (union)
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <memory>
#include <format>
#include <vector>
#include "Types.h"
#include "toolbox/SymbolTable.h"

namespace DLS {

    // ------------------------------------------
    // Value
    // ------------------------------------------
    enum class ValueType: U8 {
        None,
        Double,
        Long,
        Pointer,
        String,
        Function,
        Array
    };
    //
    struct alignas(16) Value {
        union {
            F64 d;
            S64 l;
            void* p;
            U32 strId;

        } ;
        U32 flags = 0;      // 4 byte flags can be used to type something like pointers
        ValueType type = ValueType::None; // 1 byte
        U8 subType = 0;
        U16 marshallId = 0; // for structs field access

        // ------------------
        Value() { type = ValueType::None;  }
        Value(F64 val) { type = ValueType::Double; d = val; }
        Value(S32 val) { type = ValueType::Long; l = (S64)val; }
        Value(U32 val) { type = ValueType::Long; l = (S64)val; }
        Value(U8 val)  { type = ValueType::Long; l = (S64)val; }
        Value(S64 val) { type = ValueType::Long; l = val; }
        Value(size_t val) { type = ValueType::Long; l = (S64)val; }
        Value(std::string val) { type = ValueType::String; strId = StringTable::insert(val); }
        Value(void* val) { type = ValueType::Pointer; p = val; }
        Value(void* val, U8 val_subtype) { type = ValueType::Pointer; p = val; subType = val_subtype;}
        // ------------------
        inline bool isDouble() const{ return type == ValueType::Double; }
        inline bool isFloat() const{ return type == ValueType::Double; }
        inline bool isLong() const{ return type == ValueType::Long; }
        inline bool isInt() const{ return type == ValueType::Long; }
        inline bool isPointer() const { return type == ValueType::Pointer; }
        inline bool isPointerSubType(U8 val_subtype) const { return type == ValueType::Pointer && subType == val_subtype; }
        inline bool isPointerMarshall(U16 marshall_id) const { return type == ValueType::Pointer && marshallId == marshall_id; }
        inline bool isString() const{ return type == ValueType::String; }
        inline bool isFunction() const{ return type == ValueType::Function; }
        inline bool isArray() const{ return type == ValueType::Array; }
        // ------------------
        inline F64 getFastDouble() { return d;}
        // inline F64 getDouble() const {
        //     if (type == ValueType::Double) [[likely]] return d;
        //     else if (type == ValueType::Long || type == ValueType::Array) return static_cast<F64>(l);
        //     else if (type == ValueType::Pointer) return (p != nullptr) ? 1.0 : 0.0;
        //     else return 0.0;
        // }
        FORCE_NOINLINE F64 getDoubleSlowPath(ValueType type, S64 payload) const {
            if (type == ValueType::Array)
                return static_cast<F64>(payload);
            if (type == ValueType::Pointer && payload != 0)
                return 1.0;
            return 0.0;
        }

        FORCE_INLINE F64 getDouble() const {
                if (type == ValueType::Double) [[likely]] return d;
                else if (type == ValueType::Long) return static_cast<F64>(l);
                else return getDoubleSlowPath(type, l);
            // return (type == ValueType::Double) ? d : getDoubleSlowPath(type, l);
        }

        // ------------------
       inline S64 getFastLong() const { return l; }

       FORCE_NOINLINE S64 getLongSlowPath(ValueType type, S64 payload_l, F64 payload_d) const {
           if (type == ValueType::Double) {
               return static_cast<S64>(payload_d);
           }
           if (type == ValueType::Array) {
               return static_cast<S64>(payload_l);
           }
           if (type == ValueType::Pointer) {
               return (payload_l != 0) ? 1 : 0; // p != nullptr
           }
           return 0;
       }

       FORCE_INLINE S64 getLong() const {
           if (type == ValueType::Long) [[likely]] {
               return l;
           }
           return getLongSlowPath(type, l, d);
       }

       // inline S64 getLong() const {
       //      if (type == ValueType::Long) [[likely]] return l;
       //      else if (type == ValueType::Double)  return static_cast<S64>(d);
       //      else if (type == ValueType::Array)  return l;
       //      else if (type == ValueType::Pointer) return (p != nullptr) ? 1 : 0;
       //      else return 0;
       //  }
        // ------------------
        inline F32 getFloat() const { return static_cast<F32>( getDouble()); }
        inline U8 getU8()     const { return static_cast<U8>( getLong()); }
        inline U32 getUInt()  const { return static_cast<U32>( getLong()); }
        inline S32 getInt()   const { return static_cast<S32>( getLong()); }
        inline bool getBool() const { return  getLong() != 0; }
        // ------------------
        inline const std::string& getString() const {
            if (type == ValueType::String) [[likely]] return StringTable::get(strId);
            static const std::string empty = "";
            return empty;
        }
        // ------------------
        inline void * getPointer() const {
            if (type == ValueType::Pointer) [[likely]] return p;
            else return nullptr;
        }

        // ------------------
        inline const std::string toString() const {
            switch (type) {
                case ValueType::Double: return std::to_string(d);
                case ValueType::Long: return std::to_string(l);
                case ValueType::String: return getString();
                case ValueType::Pointer: return  std::format("{:p}", static_cast<void*>(p));
                case ValueType::Array: return  "*Array*";
                case ValueType::Function: return  "*Function*";
                default:
                    static const std::string empty = "";
                    return empty;
            }
        }


    }; //Value

} //namespace

