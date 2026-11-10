//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Marshall structs ...
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <unordered_map>
#include "Value.h"
#include "Globals.h"

namespace DLS::Marshall {


    class Translator {
    public:
        U16 mMashallId = 0;
        virtual ~Translator() = default;

        virtual bool saveValue(const Value& pointerValue, U32 fieldSymbolId, const Value& src) = 0;
        virtual bool loadValue(const Value& pointerValue, U32 fieldSymbolId, Value& dest) = 0;
        virtual bool saveArrayValue(const Value& pointerValue, const std::vector<Value>& arr) = 0;
        virtual bool loadArrayValue(const Value& pointerValue, std::vector<Value>& outArr) const = 0;
        virtual bool saveInstance(const Value& pointerValue, const void* srcInstance) = 0;
        virtual bool loadInstance(const Value& srcValue, void* destInstance) const = 0;
    };
    // --------------------------------------------------------------------------------------------
    inline std::vector<Translator*> TranslatorMap;
    // --------------------------------------------------------------------------------------------
    template <typename StructType>
    class GenericStructTranslator : public Translator {
    private:
        struct FieldInfo {
            size_t offset;
            size_t arrayIndex;
            std::function<void(void* fieldPtr, const Value& src)> saveFn;
            std::function<void(const void* fieldPtr, Value& dest)> loadFn;
        };
        std::unordered_map<U32, FieldInfo> mFields;
        std::vector<FieldInfo> mFieldsByIndex;


    public:
        template <typename MemberType>
        void registerField(const char* name, MemberType StructType::* fieldTarget) {
            U32 symbolId = DLS::SymbolTable::insert(name);
            size_t offset = reinterpret_cast<size_t>(&(static_cast<StructType*>(nullptr)->*fieldTarget));
            size_t currentLength = mFieldsByIndex.size();

            FieldInfo info;
            info.offset = offset;
            info.arrayIndex = currentLength;

            info.saveFn = [](void* fieldPtr, const Value& src) {
                MemberType* m_ptr = reinterpret_cast<MemberType*>(fieldPtr);

                if constexpr (std::is_same_v<MemberType, double>) {
                    *m_ptr = src.getDouble();
                } else if constexpr (std::is_same_v<MemberType, int64_t>) {
                    *m_ptr = src.getLong();
                } else if constexpr (std::is_same_v<MemberType, float>) {
                    *m_ptr = src.getFloat();
                } else if constexpr (std::is_same_v<MemberType, int>) {
                    *m_ptr = src.getInt();
                } else if constexpr (std::is_same_v<MemberType, uint32_t>) {
                    *m_ptr = src.getUInt();
                } else if constexpr (std::is_same_v<MemberType, uint8_t>) {
                    *m_ptr = src.getU8();
                } else if constexpr (std::is_same_v<MemberType, bool>) {
                    *m_ptr = src.getBool();
                }
            };

            info.loadFn = [](const void* fieldPtr, Value& dest) {
                const MemberType* m_ptr = reinterpret_cast<const MemberType*>(fieldPtr);

                if constexpr (std::is_same_v<MemberType, double>) {
                    dest = Value(*m_ptr);
                } else if constexpr (std::is_same_v<MemberType, int64_t>) {
                    dest = Value(*m_ptr);
                } else if constexpr (std::is_same_v<MemberType, float>) {
                    dest = Value(*m_ptr);
                } else if constexpr (std::is_same_v<MemberType, uint32_t>) {
                    dest = Value(*m_ptr);
                } else if constexpr (std::is_same_v<MemberType, uint8_t>) {
                    dest = Value(*m_ptr);
                } else if constexpr (std::is_same_v<MemberType, int>) {
                    dest = Value(*m_ptr);
                } else if constexpr (std::is_same_v<MemberType, bool>) {
                    dest = Value(*m_ptr);
                }
            };

            mFields[symbolId] = info;
            mFieldsByIndex.push_back(info);
        }
        // ---------------------------------------------------------------------
        // not in use !
        // template <typename MemberType>
        // void registerNestedField(const char* name, MemberType StructType::* fieldTarget, U16 nestedMarshallId) {
        //     U32 symbolId = DLS::SymbolTable::insert(name);
        //     size_t offset = reinterpret_cast<size_t>(&(static_cast<StructType*>(nullptr)->*fieldTarget));
        //
        //     FieldInfo info;
        //     info.offset = offset;
        //     info.arrayIndex = mFieldsByIndex.size();
        //
        //     info.saveFn = [nestedMarshallId](void* fieldPtr, const Value& src) {
        //         Translator* subTranslator = TranslatorMap[nestedMarshallId];
        //         if (!subTranslator) return;
        //
        //         Value nestedPointerValue;
        //         nestedPointerValue = Value(fieldPtr);
        //
        //         if (src.type == DLS::ValueType::Array) {
        //             auto subArray = vm.Variables.getArrayByMasterIndex((size_t)src.getFastLong());
        //             subTranslator->saveArrayValue(nestedPointerValue, subArray);
        //         }
        //     };
        //
        //     // fixme save
        //
        //     mFields[symbolId] = info;
        //     mFieldsByIndex.push_back(info);
        // }
        // ---------------------------------------------------------------------
        bool saveValue(const Value& pointerValue, U32 fieldSymbolId, const Value& src) override {
            auto it = mFields.find(fieldSymbolId);
            if (it == mFields.end()) return false;

            StructType* instance = reinterpret_cast<StructType*>(pointerValue.getPointer());
            if (!instance) return false;

            void* fieldAddress = reinterpret_cast<char*>(instance) + it->second.offset;
            it->second.saveFn(fieldAddress, src);

            return true;
        }
        // ---------------------------------------------------------------------

        bool loadValue(const Value& pointerValue, U32 fieldSymbolId, Value& dest) override {
            auto it = mFields.find(fieldSymbolId);
            if (it == mFields.end()) return false;

            StructType* instance = reinterpret_cast<StructType*>(pointerValue.getPointer());
            if (!instance) return false;
            const void* fieldAddress = reinterpret_cast<const char*>(instance) + it->second.offset;

            it->second.loadFn(fieldAddress, dest);
            return true;
        }
        // ---------------------------------------------------------------------

        bool saveArrayValue(const Value& pointerValue, const std::vector<Value>& arr) {
            if (arr.size() < mFieldsByIndex.size()) return false;
            StructType* instance = reinterpret_cast<StructType*>(pointerValue.getPointer());
            if (!instance) return false;

            for (size_t i = 0; i < mFieldsByIndex.size(); ++i) {
                const auto& field = mFieldsByIndex[i];
                void* fieldAddress = reinterpret_cast<char*>(instance) + field.offset;
                field.saveFn(fieldAddress, arr[i]);
            }
            return true;
        }
        // ---------------------------------------------------------------------
        bool loadArrayValue(const Value& pointerValue, std::vector<Value>& outArr) const {
            const StructType* instance = reinterpret_cast<const StructType*>(pointerValue.getPointer());
            if (!instance) return false;
            outArr.resize(mFieldsByIndex.size());
            for (size_t i = 0; i < mFieldsByIndex.size(); ++i) {
                const auto& field = mFieldsByIndex[i];
                const void* fieldAddress = reinterpret_cast<const char*>(instance) + field.offset;
                field.loadFn(fieldAddress, outArr[i]);
            }
            return true;
        }
        // ---------------------------------------------------------------------
        bool saveInstance(const Value& pointerValue, const void* srcInstance) override {
            StructType* dest = reinterpret_cast<StructType*>(pointerValue.getPointer());
            if (!dest || !srcInstance) return false;
            const StructType* src = static_cast<const StructType*>(srcInstance);
            *dest = *src;
            return true;
        }

        // ---------------------------------------------------------------------
        bool loadInstance(const Value& srcValue, void* destInstance) const override {
            if (!destInstance) return false;

            if (srcValue.isPointerMarshall(mMashallId)) [[likely]]{
                StructType* dest = static_cast<StructType*>(destInstance);
                const StructType* src = static_cast<const StructType*>(srcValue.getPointer());
                *dest = *src;
                return true;
            }

            // if (srcValue.isArray()) {
            //     const std::vector<Value>& arr = arrayResolver(srcValue);
            //     if (arr.size() < mFieldsByIndex.size()) return false;
            //     StructType* instance = static_cast<StructType*>(destInstance);
            //     for (size_t i = 0; i < mFieldsByIndex.size(); ++i) {
            //         const auto& field = mFieldsByIndex[i];
            //         void* fieldAddress = reinterpret_cast<char*>(instance) + field.offset;
            //         field.saveFn(fieldAddress, arr[i]);
            //     }
            //     return true;
            // }

            return false;
        }

    };


// ----------------------------------------------------------------------------


inline U16 RegisterMarshall(Translator* trans) {
    if (TranslatorMap.size() == 0) TranslatorMap.push_back(nullptr); //dont want a 0
    U16 result = (U16)TranslatorMap.size();
    trans->mMashallId = result;
    TranslatorMap.push_back(trans);
    return result;
}

inline Translator* getTranslator(const Value& pointerValue) {
    if ( !pointerValue.isPointer()
        || pointerValue.marshallId == 0
        || pointerValue.marshallId >= TranslatorMap.size()) [[unlikely]]
        return nullptr;
    return TranslatorMap[pointerValue.marshallId];
}


} //namespace
