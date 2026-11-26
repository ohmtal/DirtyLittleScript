#include <SDL3/SDL.h>
#include "BaseFlux/Main.h"
#include "BaseFlux/Draw.h"

//using lib!  #define IMPLEMENT_DLS
#include "DirtyLittleScript.h"

#include "DLSBaseFlux.h"
#include "SDLTypes.h"


extern BaseFlux::Main app;
extern SDL_Point gMousePos;
typedef SDL_Color Color;

namespace DLS {
    extern void RegisterMathFunctions();
    extern void RegisterStringFunctions();
}

namespace DLS::Flux {

    extern void RegisterFluxFunctions();
    // =========================================================================
    size_t TapePos = 0;
    size_t ArrayCount = 0;
    size_t GlobalsCount = 0;
    size_t StackCount = 0;
    size_t GarbageCount = 0;

    std::string currentFileName = "";

    S64 setupEventVarRegister = -1;
    DLS::S64 initEventVarRegister = -1;
    DLS::S64 updateEventVarRegister = -1;
    DLS::S64 renderEventVarRegister = -1;
    DLS::S64 shutDownEventVarRegister = -1;
    DLS::S64 keyEventVarRegister = -1;
    DLS::S64 var_mouseX, var_mouseY, var_eventType,var_mod, var_key, var_button, var_frametime
        , var_mouseWheelX, var_mouseWheelY, var_updatedeltatime;

    DLS::S64 mouseEventVarRegister = -1;

    DLS::S64 EventVarRegister = -1;

    // -----------------------------------------------------------------------------
    void RegisterGlobals() {
        auto REG_GLOBAL = [](S64& varRegister, std::string varname) {
            varRegister = fluxVM.Variables.reserveGlobal(DLS::SymbolTable::insert( varname ));
        };

        REG_GLOBAL( setupEventVarRegister, "OnSetup" );
        REG_GLOBAL( initEventVarRegister , "OnInit" );
        REG_GLOBAL( updateEventVarRegister , "OnUpdate" );
        REG_GLOBAL( renderEventVarRegister, "OnRender");
        REG_GLOBAL( keyEventVarRegister , "OnKey" );
        REG_GLOBAL( shutDownEventVarRegister , "OnShutdown" );
        REG_GLOBAL( mouseEventVarRegister , "OnMouse" );
        REG_GLOBAL( var_mouseX      , "event::mx" );
        REG_GLOBAL( var_mouseY      , "event::my" );
        REG_GLOBAL( var_eventType   , "event::type" );
        REG_GLOBAL( var_key         , "event::key" );
        REG_GLOBAL( var_mod         , "event::mod" );
        REG_GLOBAL( var_button      , "event::button" );
        REG_GLOBAL( var_mouseWheelX , "event::wheelx" );
        REG_GLOBAL( var_mouseWheelY , "event::wheely" );

        REG_GLOBAL( var_frametime   , "flux::ft" );
        REG_GLOBAL( var_updatedeltatime   , "flux::dt" );

    }
    // -----------------------------------------------------------------------------
    void OnSetup() {
        const DLS::Value& funcVar = fluxVM.Variables.getGlobal(setupEventVarRegister);
        if (!fluxVM.PlaceTapeAtScriptFunction(funcVar)) return;
        fluxVM.run();

        DLS::Value val;
        if (fluxVM.Variables.getGlobalVariableValueRef("flux::EnableVSync", val))
            app.getSettings().EnableVSync = val.getBool();

        if (fluxVM.Variables.getGlobalVariableValueRef("flux::Company", val))
            app.getSettings().Company = val.getString();

        if (fluxVM.Variables.getGlobalVariableValueRef("flux::Caption", val))
            app.getSettings().Caption = val.getString();

        if (fluxVM.Variables.getGlobalVariableValueRef("flux::Version", val))
            app.getSettings().Version = val.getString();

        if (fluxVM.Variables.getGlobalVariableValueRef("flux::SoundPathAppend", val))
            app.getSettings().SoundPathAppend = val.getString();

        if (fluxVM.Variables.getGlobalVariableValueRef("flux::TexturePathAppend", val))
            app.getSettings().TexturePathAppend = val.getString();

        if (fluxVM.Variables.getGlobalVariableValueRef("flux::sdlWindowFlagsOverwrite", val))
            app.getSettings().sdlWindowFlagsOverwrite = (Uint64)val.getLong();

        if (fluxVM.Variables.getGlobalVariableValueRef("flux::clearColor", val)) {
            SDL_Color col;
            if (MapArrayColor(val,col))
                app.getSettings().clearColor = col;
        }

    }
    // -----------------------------------------------------------------------------
    // Events
    // -----------------------------------------------------------------------------
    void OnInit() {
        const DLS::Value& funcVar = fluxVM.Variables.getGlobal(initEventVarRegister);
        if (!fluxVM.PlaceTapeAtScriptFunction(funcVar)) return;
        fluxVM.run();
    }
    // -----------------------------------------------------------------------------
    void OnUpdate(const float dt) {
        fluxVM.Variables.saveGlobal(var_updatedeltatime, DLS::Value(dt));

        const DLS::Value& funcVar = fluxVM.Variables.getGlobal(updateEventVarRegister);
        if (!fluxVM.PlaceTapeAtScriptFunction(funcVar)) return;
        fluxVM.run();
    }
    // -----------------------------------------------------------------------------
    void OnShutdown() {
        const DLS::Value& funcVar = fluxVM.Variables.getGlobal(shutDownEventVarRegister);
        if (!fluxVM.PlaceTapeAtScriptFunction(funcVar)) return;
        fluxVM.run();
    }
    // -----------------------------------------------------------------------------
    void OnRender() {
        // example:
        // OnRender = function; flux::DrawFPS 50 50 5;;

        if (renderEventVarRegister < 0) return;

        fluxVM.Variables.saveGlobal(var_frametime, DLS::Value(BaseFlux::getFrameTime() * 100.0));

        const DLS::Value& funcVar = fluxVM.Variables.getGlobal(renderEventVarRegister);
        if (!fluxVM.PlaceTapeAtScriptFunction(funcVar)) return;
        fluxVM.run();

        //NOTE debug/stats:
        ArrayCount = fluxVM.Variables.mArrays.size();
        GlobalsCount = fluxVM.Variables.mGlobalsRegister.size();
        StackCount = fluxVM.STACK_SIZE();
        GarbageCount = DLS::GarbageList.objects.size();
    }

    // -----------------------------------------------------------------------------
    void OnKey(SDL_Keycode key, DLS::U32 EventType, SDL_Keymod mod) {
        if (keyEventVarRegister < 0) return;

        fluxVM.Variables.saveGlobal(var_key, DLS::Value(key));
        fluxVM.Variables.saveGlobal(var_eventType, DLS::Value(EventType));
        fluxVM.Variables.saveGlobal(var_mod, DLS::Value(mod));

        const DLS::Value& funcVar = fluxVM.Variables.getGlobal(keyEventVarRegister);
        if (!fluxVM.PlaceTapeAtScriptFunction(funcVar)) return;
        fluxVM.run();
    }

    // -----------------------------------------------------------------------------
    // only set the var no event
    void OnMouseMotion(const SDL_MouseMotionEvent& mmev) {
        fluxVM.Variables.saveGlobal(var_mouseX, DLS::Value(mmev.x));
        fluxVM.Variables.saveGlobal(var_mouseY, DLS::Value(mmev.y));
    }

    // -----------------------------------------------------------------------------
    void OnMouseWheel(const SDL_MouseWheelEvent& mWheelEvent) {
        fluxVM.Variables.saveGlobal(var_mouseWheelX, DLS::Value(mWheelEvent.x));
        fluxVM.Variables.saveGlobal(var_mouseWheelY, DLS::Value(mWheelEvent.y));
        fluxVM.Variables.saveGlobal(var_eventType, DLS::Value((DLS::U32)SDL_EVENT_MOUSE_WHEEL));
        if (mouseEventVarRegister < 0) return;
        const DLS::Value& funcVar = fluxVM.Variables.getGlobal(mouseEventVarRegister);
        if (!fluxVM.PlaceTapeAtScriptFunction(funcVar)) return;
        fluxVM.run();
    }

    // -----------------------------------------------------------------------------
    void OnMouse(DLS::U8 button,  DLS::U32 EventType) {
        fluxVM.Variables.saveGlobal(var_button, DLS::Value(button));
        fluxVM.Variables.saveGlobal(var_eventType, DLS::Value(EventType));


        if (mouseEventVarRegister < 0) return;
        const DLS::Value& funcVar = fluxVM.Variables.getGlobal(mouseEventVarRegister);
        if (!fluxVM.PlaceTapeAtScriptFunction(funcVar)) return;
        fluxVM.run();
    }
    // -----------------------------------------------------------------------------
    size_t getTapePos() { return TapePos;}
    size_t getArrayCount() { return ArrayCount;}
    size_t getGlobalCount() { return GlobalsCount;}
    size_t getStackCount() { return StackCount;}
    size_t getGarbageCount() { return GarbageCount;}
    //  -----------------------------------------------------------------------------
    static void DLS_Logger(DLS::Tools::LogLevel level, const char* message){
        switch (level) {
            case DLS::Tools::LogLevel::warning: SDL_Log("[warn] %s",  message); break;
            case DLS::Tools::LogLevel::error:   SDL_Log("[error] %s",  message); break;
            case DLS::Tools::LogLevel::info:    SDL_Log("[info] %s",  message); break;
            default:SDL_Log("%s",  message); break;
        }
    }
    // -------------------------------------------------------------------------
    void Init (int argc, char* argv[]) {
        DLS::Init(argc,argv,DLS::Flux::DLS_Logger);
        DLS::RegisterMathFunctions();
        DLS::RegisterStringFunctions( );

        RegisterFluxFunctions();
        RegisterGlobals();
        RegisterSDLTypes();

    }
    // -------------------------------------------------------------------------
    bool Run (const std::string& command) {
        using namespace DLS;
        if (!Compile(fluxVM,command)) return false;
        fluxVM.run();
        TapePos = fluxVM.tape.size();

        return true;
    }
    // -------------------------------------------------------------------------
    const std::string& getScriptFile()  { return currentFileName; }
    // -------------------------------------------------------------------------
    void DumpTape() {
        fluxVM.dumpTape();
    }
    // -------------------------------------------------------------------------
    bool RunFile(const std::string& fileName, bool ResetTape ) {
        currentFileName = fileName;
        if (ResetTape) {
            fluxVM.tape.clear();
            fluxVM.pc = 0;
            fluxVM.Variables.reset();

            ArrayCount = 0;
            GlobalsCount = 0;
            StackCount = 0;

            DLS::GarbageList.Clear();
            RegisterGlobals();
        }
        if (!Run(DLS::loadScript(fileName))) return false;
        if (ResetTape) DLS::Flux::OnInit();
        return true;
    }

}
