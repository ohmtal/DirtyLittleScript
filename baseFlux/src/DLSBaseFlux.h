#pragma once
#include <SDL3/SDL.h>
#include "core/VM.h"
namespace DLS::Flux {
    // =========================================================================
    inline VM fluxVM;
    // =========================================================================
    void Init (int argc, char* argv[]);
    bool Run(const std::string& command);
    size_t getTapePos();
    size_t getArrayCount();
    size_t getGlobalCount();
    size_t getStackCount();
    size_t getGarbageCount();
    void OnSetup();
    void OnInit();
    void OnUpdate(const float dt);
    void OnShutdown();
    void OnRender();
    void DumpTape();
    bool RunFile(const std::string& fileName, bool ResetTape );
    void OnKey(SDL_Keycode key, DLS::U32 EventType, SDL_Keymod mod);
    void OnMouse( DLS::U8 button,  DLS::U32 EventType);
    void OnMouseMotion(const SDL_MouseMotionEvent& mmev);
    void OnMouseWheel(const SDL_MouseWheelEvent& mWheelEvent);
    const std::string& getScriptFile();

}
