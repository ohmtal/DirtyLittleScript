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

do not use me :P

#include <vector>
#include "Value.h"
#include "Marshall.h"
#include "FunctionMap.h"
#include "toolbox/ArenaAllocator.h"

namespace DLS {
    // -------------------------------------------------------------------------
    struct ObjectMarshall;
    using namespace FunctionMap;
    inline std::vector<ObjectMarshall*> ObjectsMap;
    inline ArenaAllocator ObjectsArena;


    // -------------------------------------------------------------------------
    // templates are cool but cause brain damage ;)
    #define CREATE_OBJECT_MARSHALL(translator) (new DLS::ObjectMarshall(translator))
    // #define OBJECT_FIELD(objectMarshallPtr,fieldName,fieldTarget) (objectMarshallPtr->mTranslator->registerField(fieldName, fieldTarget))
    // -------------------------------------------------------------------------
    class ObjectMarshall {
        U16 mId = 0;
        Marshall::IStructTranslator* mTranslator;
    public:
        FuncLookupMap mMethods; //<<< NOT need also the object .. or i look at ElfScript Trampolin
        DescLookupMap mMethodHelp;

        inline U16 getId() { return mId;}

        ObjectMarshall( Marshall::IStructTranslator* translator) {
            mTranslator = translator;
            ObjectsArena.Add(mTranslator);
            ObjectsArena.Add(this);
            mId = static_cast<U16>(ObjectsMap.size());
        };
        void registerMethod(const std::string& name, CallBack cb, std::string desc = "") {
            uint32_t symId = SymbolTable::insert(name);
            mMethods[symId ] = cb;
            if (!desc.empty()) {
                uint32_t strId = StringTable::insert(desc);
                mMethodHelp[symId] = strId;
            }
        }
    };

} // namespace
