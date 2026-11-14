//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// SDLTypes
//-----------------------------------------------------------------------------
#pragma once
#include "SDL3/SDL.h"
#include "core/Value.h"

namespace DLS::Flux {
    using namespace DLS;


    typedef struct SDL_FPoint3
    {
        float x;
        float y;
        float z;
    } SDL_FPoint3;

    inline U16 SDL_FPointID = 0;
    inline U16 SDL_FPoint3ID = 0;
    inline U16 SDL_FRectID = 0;
    inline U16 SDL_ColorID = 0;

    inline bool isPoint(const Value& value) { return value.isPointerMarshall(SDL_FPointID); }
    inline bool isPoint3(const Value& value) { return value.isPointerMarshall(SDL_FPoint3ID); }
    inline bool isRect(const Value& value) { return value.isPointerMarshall(SDL_FRectID); }
    inline bool isColor(const Value& value) { return value.isPointerMarshall(SDL_ColorID); }


    inline DLS::ArenaAllocator marshallArena;
    void RegisterSDLTypes();

    bool MapArrayFRect( Value& value, SDL_FRect& rect);
    bool MapArrayColor( Value& value, SDL_Color& color);
    bool MapArrayFPoint( DLS::Value& value, SDL_FPoint& point);

};
