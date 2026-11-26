//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Objects - i restarted from DreiZehn making a small script but miss Objects
//           Struct field translator is in place, so i have to wrap it a bit
//-----------------------------------------------------------------------------
// NOTE: unfinished i need so save the register in the object and need a other
//       callback to the object itself .. so this if far away from finished.
//-----------------------------------------------------------------------------
#pragma once


#include <vector>
#include "Value.h"
#include "Marshall.h"
#include "FunctionMap.h"
#include "toolbox/ArenaAllocator.h"

namespace DLS {
    // -------------------------------------------------------------------------
    // // struct ObjectMarshall;
    // // using namespace FunctionMap;
    // // inline std::vector<ObjectMarshall*> ObjectsMap;

    inline ArenaAllocator gObjectsArena;
    struct GenericObject {
        static  DLS::Marshall::IStructTranslator* sTranslator; // = nullptr;
        static  U32 sMarshallId;

        GenericObject() {
            gObjectsArena.Add(this);
        }
//FIXME loadValueByRegister / saveValueByRegister need a Value& can not pass this!
        // bool getFieldByRegister(size_t registerIndex, Value& result) {
        //     if (!sTranslator) return false;
        //     return sTranslator->loadValueByRegister(this,registerIndex,result);
        // };
        // bool getFieldBySymId(U32 fieldSymId, Value& result) {
        //     if (!sTranslator) return false;
        //     size_t registerIndex;
        //     sTranslator->getFieldRegister(fieldSymId, registerIndex);
        //     return sTranslator->loadValueByRegister(this,registerIndex,result);
        // };
        //
        // bool setFieldByRegister(size_t registerIndex, const Value& value) {
        //     if (!sTranslator) return false;
        //     sTranslator->saveValueByRegister(this, registerIndex, value);
        // }
        // bool setFieldBySymId(U32 symId, const Value& value) {
        //
        // };
        //
        virtual bool callMethod(U32 symId, VM&, ValueSpan, Value&) = 0;

        static void initFields() {}

    };



    // -------------------------------------------------------------------------
    // auto* frectTranslator = CREATE_TRANSLATOR(SDL_FRect);
    // marshallArena.Add(frectTranslator);
    // frectTranslator->registerField("x", &SDL_FRect::x);
    // frectTranslator->registerField("y", &SDL_FRect::y);
    // frectTranslator->registerField("w", &SDL_FRect::w);
    // frectTranslator->registerField("h", &SDL_FRect::h);
    // SDL_FRectID = RegisterStructTranslator(frectTranslator);
    // FunctionMap::RegisterFunction("Rect::new", [](VM& vm, ValueSpan args, Value& ret) -> bool {
    //     SDL_FRect* p = new SDL_FRect();
    //     DLS::GarbageList.Add(p);
    //     ret = Value(p);
    //     ret.marshallId = SDL_FRectID;
    //     if (args.size() > 0 && args[0].isArray()) {
    //         DLS::Marshall::IStructTranslator* tl = DLS::Marshall::getStructTranslator(ret);
    //         if (tl) tl->saveArrayValue(ret,vm.Variables.getArrayByMasterIndex((size_t)args[0].getFastLong()));
    //     }
    //     return true;
    // }, "Create a Rect (x,y,w,h).");
    // -------------------------------------------------------------------------






    // using MethodCallBack =  std::function< bool (VM&, BaseObject&, ValueSpan, Value& )>;
    // using MethodLookupMap = std::unordered_map<uint32_t, MethodCallBack>;


    // // // -------------------------------------------------------------------------
    // // // templates are cool but cause brain damage ;)
    // // #define CREATE_OBJECT_MARSHALL(translator) (new DLS::ObjectMarshall(translator))
    // // // #define OBJECT_FIELD(objectMarshallPtr,fieldName,fieldTarget) (objectMarshallPtr->mTranslator->registerField(fieldName, fieldTarget))
    // // // -------------------------------------------------------------------------
    // // class ObjectMarshall {
    // //     U16 mId = 0;
    // //     Marshall::IStructTranslator* mTranslator;
    // // public:
    // //     FuncLookupMap mMethods; //<<< NOT! => need also the object .. or i look at ElfScript Trampolin
    // //     DescLookupMap mMethodHelp;
    // //
    // //     inline U16 getId() { return mId;}
    // //
    // //     ObjectMarshall( Marshall::IStructTranslator* translator) {
    // //         mTranslator = translator;
    // //         ObjectsArena.Add(mTranslator);
    // //         ObjectsArena.Add(this);
    // //         mId = static_cast<U16>(ObjectsMap.size());
    // //     };
    // //     void registerMethod(const std::string& name, CallBack cb, std::string desc = "") {
    // //         uint32_t symId = SymbolTable::insert(name);
    // //         mMethods[symId ] = cb;
    // //         if (!desc.empty()) {
    // //             uint32_t strId = StringTable::insert(desc);
    // //             mMethodHelp[symId] = strId;
    // //         }
    // //     }
    // // };

} // namespace
