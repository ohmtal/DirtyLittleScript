//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// SDLTypes
//-----------------------------------------------------------------------------

#include "SDL3/SDL.h"
#include "core/Marshall.h" //NOTE: DLS: Object Test
#include "toolbox/ArenaAllocator.h" //NOTE: DLS: Object Test

#include "DLSBaseFlux.h"
#include "SDLTypes.h"

// #include "core/Objects.h"

namespace DLS::Flux {
    // -----------------------------------------------------------------------------
    bool MapArrayFRect( DLS::Value& value, SDL_FRect& rect) {

        if (isRect(value)) {
            rect = *(static_cast<SDL_FRect*>(value.getPointer()));
            return true;
        }

        if (value.type != DLS::ValueType::Array) return false;
        auto arr = fluxVM.Variables.getArrayByMasterIndex((size_t)value.getFastLong());
        if (arr.size() < 4) return false;
        rect.x = arr[0].getFloat();
        rect.y = arr[1].getFloat();
        rect.w = arr[2].getFloat();
        rect.h = arr[3].getFloat();
        if (rect.w == 0.f || rect.h == 0.f) return false;
        return true;
    }
    // -----------------------------------------------------------------------------
    bool MapArrayColor( DLS::Value& value, SDL_Color& color) {

        if (isColor(value)) {
            color = *(static_cast<SDL_Color*>(value.getPointer()));
            return true;
        }

        if (value.type != DLS::ValueType::Array) return false;
        auto arr = fluxVM.Variables.getArrayByMasterIndex((size_t)value.getFastLong());
        if (arr.size() < 3) return false;
        color.r = arr[0].getU8();
        color.g = arr[1].getU8();
        color.b = arr[2].getU8();
        if (arr.size() > 3) color.a = arr[3].getU8();
        else color.a = 255;
        return true;
    }
    // -----------------------------------------------------------------------------
    bool MapArrayFPoint( DLS::Value& value, SDL_FPoint& point) {

        if (isPoint(value)) {
            point = *(static_cast<SDL_FPoint*>(value.getPointer()));
            return true;
        }

        if (value.type != DLS::ValueType::Array) return false;
        auto arr = fluxVM.Variables.getArrayByMasterIndex((size_t)value.getFastLong());
        if (arr.size() < 2) return false;
        point.x = arr[0].getFloat();
        point.y = arr[1].getFloat();
        return true;
    }
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    void RegisterSDLTypes() {

        // ---------------------------------------------------------------------
        // SDL_FRect
        // ---------------------------------------------------------------------
        // NOTE unfinished TEST:
        // {
        //     auto* ft = CREATE_TRANSLATOR(SDL_FRect);
        //     ft->registerField("x", &SDL_FRect::x);
        //     ft->registerField("y", &SDL_FRect::y);
        //     ft->registerField("w", &SDL_FRect::w);
        //     ft->registerField("h", &SDL_FRect::h);
        //     auto obj = CREATE_OBJECT_MARSHALL(ft);
        //     obj->registerMethod("print", [](VM&,ValueSpan,Value&) -> bool {
        //         // Tools::printf("") - need to translate the values here... like in the VM but
        //         return true;
        //     }," Print the rectangle to console" );
        //    //FIXME  SDL_FRectID = obj->getId();
        // }


        // new CREATE_TRANSLATOR macro for: `new DLS::Marshall::GenericStructTranslator<SDL_FRect>();`
        auto* frectTranslator = CREATE_TRANSLATOR(SDL_FRect);
        marshallArena.Add(frectTranslator);
        frectTranslator->registerField("x", &SDL_FRect::x);
        frectTranslator->registerField("y", &SDL_FRect::y);
        frectTranslator->registerField("w", &SDL_FRect::w);
        frectTranslator->registerField("h", &SDL_FRect::h);
        SDL_FRectID = RegisterStructTranslator(frectTranslator);

        FunctionMap::RegisterFunction("Rect::new", [](VM&, ValueSpan args, Value& ret) -> bool {
            SDL_FRect* p = new SDL_FRect();
            DLS::GarbageList.Add(p);
            ret = Value(p);
            ret.marshallId = SDL_FRectID;
            if (args.size() > 0 && args[0].isArray()) {
                DLS::Marshall::IStructTranslator* tl = DLS::Marshall::getStructTranslator(ret);
                if (tl) tl->saveArrayValue(ret,fluxVM.Variables.getArrayByMasterIndex((size_t)args[0].getFastLong()));
            }
            return true;
        }, "Create a Rect (x,y,w,h).");

        // ---------------------------------------------------------------------
        // SDL_FPoint with marshalling
        // ---------------------------------------------------------------------
        auto* fpointTranslator = new DLS::Marshall::GenericStructTranslator<SDL_FPoint>();
        marshallArena.Add(fpointTranslator);
        fpointTranslator->registerField("x", &SDL_FPoint::x);
        fpointTranslator->registerField("y", &SDL_FPoint::y);
        SDL_FPointID = RegisterStructTranslator(fpointTranslator);

        FunctionMap::RegisterFunction("Point::new", [](VM&, ValueSpan args, Value& ret) -> bool {
            SDL_FPoint* p = new SDL_FPoint();
            DLS::GarbageList.Add(p);
            ret = Value(p);
            ret.marshallId = SDL_FPointID;
            if (args.size() > 0 && args[0].isArray()) {
                // MapArrayFPoint(args[0], *p);
                DLS::Marshall::IStructTranslator* tl = DLS::Marshall::getStructTranslator(ret);
                if (tl) tl->saveArrayValue(ret,fluxVM.Variables.getArrayByMasterIndex((size_t)args[0].getFastLong()));
            }
            return true;
        }, "Create a Point (x,y)");
        // ---------------------------------------------------------------------
        // SDL_FPoint3 with marshalling
        // ---------------------------------------------------------------------
        auto* fpoint3Translator = new DLS::Marshall::GenericStructTranslator<SDL_FPoint3>();
        marshallArena.Add(fpoint3Translator);
        fpoint3Translator->registerField("x", &SDL_FPoint3::x);
        fpoint3Translator->registerField("y", &SDL_FPoint3::y);
        fpoint3Translator->registerField("z", &SDL_FPoint3::z);
        SDL_FPoint3ID = RegisterStructTranslator(fpoint3Translator);

        FunctionMap::RegisterFunction("Point3::new", [](VM&, ValueSpan args, Value& ret) -> bool {
            SDL_FPoint3* p = new SDL_FPoint3();
            DLS::GarbageList.Add(p);
            ret = Value(p);
            ret.marshallId = SDL_FPoint3ID;
            if (args.size() > 0 && args[0].isArray()) {
                DLS::Marshall::IStructTranslator* tl = DLS::Marshall::getStructTranslator(ret);
                if (tl) tl->saveArrayValue(ret,fluxVM.Variables.getArrayByMasterIndex((size_t)args[0].getFastLong()));
            }
            return true;
        }, "Create a Point3 (x,y,z)");
        // ---------------------------------------------------------------------
        // SDL_Color with marshalling
        // ---------------------------------------------------------------------
        auto* colorTranslator = new DLS::Marshall::GenericStructTranslator<SDL_Color>();
        marshallArena.Add(colorTranslator);
        colorTranslator->registerField("r", &SDL_Color::r);
        colorTranslator->registerField("g", &SDL_Color::g);
        colorTranslator->registerField("b", &SDL_Color::b);
        colorTranslator->registerField("a", &SDL_Color::a);
        SDL_ColorID = RegisterStructTranslator(colorTranslator);

        FunctionMap::RegisterFunction("Color::new", [](VM&, ValueSpan args, Value& ret) -> bool {
            SDL_Color* p = new SDL_Color();
            DLS::GarbageList.Add(p);
            ret = Value(p);
            ret.marshallId = SDL_ColorID;
            if (args.size() > 0 && args[0].isArray()) {
                DLS::Marshall::IStructTranslator* tl = DLS::Marshall::getStructTranslator(ret);
                if (tl) tl->saveArrayValue(ret,fluxVM.Variables.getArrayByMasterIndex((size_t)args[0].getFastLong()));
            }
            return true;
        }, "Create a Color (r,g,b,a)");
        // ---------------------------------------------------------------------
    }

}// namespace DLS::Flux
