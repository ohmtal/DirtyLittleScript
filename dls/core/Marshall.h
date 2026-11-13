//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Marshall structs ...
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <functional>
#include <unordered_map>
#include "Value.h"
#include "Globals.h"

namespace DLS::Marshall {

    template <typename MemberType>
    constexpr auto doSave = [](void* fieldPtr, const Value& src) {
        MemberType* m_ptr = reinterpret_cast<MemberType*>(fieldPtr);

        if constexpr (std::is_same_v<MemberType, double>) {
            *m_ptr = src.getDouble();
        } else if constexpr (std::is_same_v<MemberType, float>) {
            *m_ptr = src.getFloat();
        } else if constexpr (std::is_same_v<MemberType, S64>) {
            *m_ptr = src.getLong();
        } else if constexpr (std::is_same_v<MemberType, S32>) {
            *m_ptr = src.getInt();
        } else if constexpr (std::is_same_v<MemberType, U32>) {
            *m_ptr = src.getUInt();
        } else if constexpr (std::is_same_v<MemberType, U8>) {
            *m_ptr = src.getU8();
        } else if constexpr (std::is_same_v<MemberType, bool>) {
            *m_ptr = src.getBool();
        } else if constexpr (std::is_same_v<MemberType, const char *>) {
            if (src.isString()) [[likely]] {
                *m_ptr = src.getString().c_str();
            } else {
                *m_ptr = StringTable::get(StringTable::insert(src.toString())).c_str();
            }
        } else if constexpr (std::is_same_v<MemberType, std::string>) {
            if (src.isString()) [[likely]] {
                *m_ptr = src.getString();
            } else {
                *m_ptr = StringTable::get(StringTable::insert(src.toString()));
            }
        }
    };

    template <typename MemberType>
    constexpr auto doLoad = [](const void* fieldPtr, Value& dest) {
        const MemberType* m_ptr = reinterpret_cast<const MemberType*>(fieldPtr);
        if constexpr (std::is_same_v<MemberType, double>) {
            dest = Value(*m_ptr);
        } else if constexpr (std::is_same_v<MemberType, float>) {
            dest = Value(*m_ptr);
        } else if constexpr (std::is_same_v<MemberType, S64>) {
            dest = Value(*m_ptr);
        } else if constexpr (std::is_same_v<MemberType, U32>) {
            dest = Value(*m_ptr);
        } else if constexpr (std::is_same_v<MemberType, U8>) {
            dest = Value(*m_ptr);
        } else if constexpr (std::is_same_v<MemberType, S32>) {
            dest = Value(*m_ptr);
        } else if constexpr (std::is_same_v<MemberType, bool>) {
            dest = Value(*m_ptr);
        } else if constexpr (std::is_same_v<MemberType, const char *>) {
            dest = Value(std::string(*m_ptr));
        } else if constexpr (std::is_same_v<MemberType, std::string>) {
            dest = Value(*m_ptr);
        }
    };



    // --------------------------------------------------------------------------------------------
    // Translator Interface
    class IStructTranslator {
    public:
        U16 mMashallId = 0;
        virtual ~IStructTranslator() = default;

        virtual bool getFieldRegister(U32 fieldSymbolId, size_t& target) = 0;

        virtual bool saveValueByRegister(const Value& pointerValue, size_t fieldRegisterId, const Value& src) = 0;
        virtual bool loadValueByRegister(const Value& pointerValue, size_t fieldRegisterId, Value& dest) = 0;

        virtual bool saveArrayValue(const Value& pointerValue, const ValueVector& arr) = 0;
        virtual bool loadArrayValue(const Value& pointerValue, ValueVector& outArr) const = 0;

        virtual bool saveInstance(const Value& pointerValue, const void* srcInstance) = 0;
        virtual bool loadInstance(const Value& srcValue, void* destInstance) const = 0;
    };
    // --------------------------------------------------------------------------------------------
    inline std::vector<IStructTranslator*> StructTranslatorMap;
    // --------------------------------------------------------------------------------------------
    template <typename StructType>
    class GenericStructTranslator : public IStructTranslator {
    private:
        struct FieldInfo {
            size_t offset;
            size_t arrayIndex;
            std::function<void(void* fieldPtr, const Value& src)> saveFn;
            std::function<void(const void* fieldPtr, Value& dest)> loadFn;
        };

        std::unordered_map<U32, size_t> mFieldLookup;
        std::vector<FieldInfo> mFieldStack;


    public:
        template <typename MemberType>
        void registerField(const char* name, MemberType StructType::* fieldTarget) {
            U32 symbolId = DLS::SymbolTable::insert(name);
            size_t offset = reinterpret_cast<size_t>(&(static_cast<StructType*>(nullptr)->*fieldTarget));

            FieldInfo info;
            info.offset = offset;
            info.arrayIndex =  mFieldStack.size();

            info.saveFn = doSave<MemberType>;
            info.loadFn = doLoad<MemberType>;

            mFieldLookup[symbolId] = info.arrayIndex;
            mFieldStack.push_back(info);
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
        //         Translator* subTranslator = StructTranslatorMap[nestedMarshallId];
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
        bool getFieldRegister(U32 fieldSymbolId, size_t& target) override {
            auto it = mFieldLookup.find(fieldSymbolId);
            if (it == mFieldLookup.end()) return false;
            target = it->second;
            return true;
        }

        bool saveValueByRegister(const Value& pointerValue, size_t fieldRegisterId, const Value& src) override
        {
            if (fieldRegisterId >= mFieldStack.size())[[unlikely]] return false;
            StructType* instance = reinterpret_cast<StructType*>(pointerValue.getPointer());
            if (!instance) return false;

            auto& info = mFieldStack[fieldRegisterId];
            void* fieldAddress = reinterpret_cast<char*>(instance) + info.offset;
            info.saveFn(fieldAddress, src);

            return true;
        }
        // ---------------------------------------------------------------------

        bool loadValueByRegister(const Value& pointerValue, size_t fieldRegisterId, Value& dest) override
        {
            if (fieldRegisterId >= mFieldStack.size())[[unlikely]] return false;
            StructType* instance = reinterpret_cast<StructType*>(pointerValue.getPointer());
            if (!instance) return false;
            auto& info = mFieldStack[fieldRegisterId];
            const void* fieldAddress = reinterpret_cast<const char*>(instance) + info.offset;

            info.loadFn(fieldAddress, dest);
            return true;
        }
        // ---------------------------------------------------------------------

        bool saveArrayValue(const Value& pointerValue, const ValueVector& arr) {
            if (arr.size() < mFieldStack.size()) return false;
            StructType* instance = reinterpret_cast<StructType*>(pointerValue.getPointer());
            if (!instance) return false;

            for (size_t i = 0; i < mFieldStack.size(); ++i) {
                const auto& field = mFieldStack[i];
                void* fieldAddress = reinterpret_cast<char*>(instance) + field.offset;
                field.saveFn(fieldAddress, arr[i]);
            }
            return true;
        }
        // ---------------------------------------------------------------------
        bool loadArrayValue(const Value& pointerValue, ValueVector& outArr) const {
            const StructType* instance = reinterpret_cast<const StructType*>(pointerValue.getPointer());
            if (!instance) return false;
            outArr.resize(mFieldStack.size());
            for (size_t i = 0; i < mFieldStack.size(); ++i) {
                const auto& field = mFieldStack[i];
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
            //     const ValueVector& arr = arrayResolver(srcValue);
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
inline U16 RegisterStructTranslator(IStructTranslator* trans) {
    if (StructTranslatorMap.size() == 0) StructTranslatorMap.push_back(nullptr); //dont want a 0
    U16 result = (U16)StructTranslatorMap.size();
    trans->mMashallId = result;
    StructTranslatorMap.push_back(trans);
    return result;
}
inline IStructTranslator* getStructTranslator(const Value& pointerValue) {
    if ( !pointerValue.isPointer()
        || pointerValue.marshallId == 0
        || pointerValue.marshallId >= StructTranslatorMap.size()) [[unlikely]]
        return nullptr;
    return StructTranslatorMap[pointerValue.marshallId];
}

// ----------------------------------------------------------------------------
// For single Variables:
// ----------------------------------------------------------------------------

struct IValueTranslator {
protected:
    std::function<void(void* fieldPtr, const Value& src)> saveFn;
    std::function<void(const void* fieldPtr, Value& dest)> loadFn;
public:
    virtual ~IValueTranslator() = default;
    virtual Value getValue() = 0;
    virtual void loadValue(Value& dest)      = 0;
    virtual void saveValue(const Value& src) = 0;
};

// --------------------------------------------------------------------------------------------
template <typename MemberType>
class ValueTranslator : public IValueTranslator {
    MemberType* m_ptr;
public:
    explicit ValueTranslator(MemberType* ptr) : m_ptr(ptr) {
        saveFn = doSave<MemberType>;
        loadFn = doLoad<MemberType>;
    }

    Value getValue()  /*override*/ {
        Value dest;
        loadFn(m_ptr, dest);
        return(dest);
    }

    void loadValue(Value& dest)  override {
        loadFn(m_ptr, dest);
    }
    void saveValue(const Value& src)   override {
        saveFn(m_ptr, src);
    }
};
// --------------------------------------------------------------------------------------------
// inline std::vector<IValueTranslator*> ValueTranslatorMap;
//
// inline U16 RegisterValueTranslator(IValueTranslator* trans) {
//     if (ValueTranslatorMap.size() == 0) ValueTranslatorMap.push_back(nullptr); //dont want a 0
//     U16 result = (U16)ValueTranslatorMap.size();
//     trans->mMashallId = result;
//     ValueTranslatorMap.push_back(trans);
//     return result;
// }
// inline IValueTranslator* getValueTranslator(const Value& pointerValue) {
//     if ( !pointerValue.isPointer()
//         || pointerValue.marshallId == 0
//         || pointerValue.marshallId >= ValueTranslatorMap.size()) [[unlikely]]
//         return nullptr;
//     return ValueTranslatorMap[pointerValue.marshallId];
// }



} //namespace
