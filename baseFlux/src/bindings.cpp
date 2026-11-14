#include <SDL3/SDL.h>
#include "BaseFlux/Main.h"
#include "BaseFlux/Draw.h"

//using lib!  #define IMPLEMENT_DLS
#include "DirtyLittleScript.h"
#include "core/Marshall.h" //NOTE: DLS: Object Test
#include "core/ArenaAllocator.h" //NOTE: DLS: Object Test



extern BaseFlux::Main app;
extern SDL_Point gMousePos;
typedef SDL_Color Color;

namespace DLS {
    extern void RegisterMathFunctions();
    extern void RegisterStringFunctions();
}

namespace DLSFlux {
    // =========================================================================
    DLS::VM vm; // let the tape grow ?!
    size_t TapePos = 0;
    size_t ArrayCount = 0;
    size_t GlobalsCount = 0;
    size_t StackCount = 0;
    size_t GarbageCount = 0;

    std::string currentFileName = "";

    DLS::S64 setupEventVarRegister = -1;
    DLS::S64 initEventVarRegister = -1;
    DLS::S64 updateEventVarRegister = -1;
    DLS::S64 renderEventVarRegister = -1;
    DLS::S64 shutDownEventVarRegister = -1;
    DLS::S64 keyEventVarRegister = -1;
    DLS::S64 var_mouseX, var_mouseY, var_eventType,var_mod, var_key, var_button, var_frametime
        , var_mouseWheelX, var_mouseWheelY, var_updatedeltatime;

    DLS::S64 mouseEventVarRegister = -1;

    DLS::S64 EventVarRegister = -1;

    DLS::ArenaAllocator marshallArena;


    typedef struct SDL_FPoint3
    {
        float x;
        float y;
        float z;
    } SDL_FPoint3;

    inline DLS::U16 SDL_FPointID = 0;
    inline DLS::U16 SDL_FPoint3ID = 0;
    inline DLS::U16 SDL_FRectID = 0;
    inline DLS::U16 SDL_ColorID = 0;

    bool isPoint(const DLS::Value& value) { return value.isPointerMarshall(SDL_FPointID); }
    bool isPoint3(const DLS::Value& value) { return value.isPointerMarshall(SDL_FPoint3ID); }
    bool isRect(const DLS::Value& value) { return value.isPointerMarshall(SDL_FRectID); }
    bool isColor(const DLS::Value& value) { return value.isPointerMarshall(SDL_ColorID); }
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    bool MapArrayFRect( DLS::Value& value, SDL_FRect& rect) {

        if (isRect(value)) {
            rect = *(static_cast<SDL_FRect*>(value.getPointer()));
            return true;
        }

        if (value.type != DLS::ValueType::Array) return false;
        auto arr = vm.Variables.getArrayByMasterIndex((size_t)value.getFastLong());
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
        auto arr = vm.Variables.getArrayByMasterIndex((size_t)value.getFastLong());
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
        auto arr = vm.Variables.getArrayByMasterIndex((size_t)value.getFastLong());
        if (arr.size() < 2) return false;
        point.x = arr[0].getFloat();
        point.y = arr[1].getFloat();
        return true;
    }
    // -----------------------------------------------------------------------------
    void RegisterGlobals() {
        setupEventVarRegister = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "OnSetup" ));
        initEventVarRegister = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "OnInit" ));
        updateEventVarRegister = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "OnUpdate" ));
        renderEventVarRegister = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "OnRender" ));
        keyEventVarRegister = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "OnKey" ));
        shutDownEventVarRegister = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "OnShutdown" ));
        mouseEventVarRegister = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "OnMouse" ));
        var_mouseX      = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "event::mx" ));
        var_mouseY      = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "event::my" ));
        var_eventType   = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "event::type" ));
        var_key         = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "event::key" ));
        var_mod         = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "event::mod" ));
        var_button      = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "event::button" ));
        var_mouseWheelX = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "event::wheelx" ));
        var_mouseWheelY = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "event::wheely" ));

        var_frametime   = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "flux::ft" ));
        var_updatedeltatime   = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( "flux::dt" ));



    }

    void OnSetup() {
        DLS::Value& funcVar = vm.Variables.getGlobal(setupEventVarRegister);
        if (!vm.PlaceTapeAtScriptFunction(funcVar)) return;
        vm.run();

        DLS::Value val;
        if (vm.Variables.getGlobalVariableValueRef("flux::EnableVSync", val))
            app.getSettings().EnableVSync = val.getBool();

        if (vm.Variables.getGlobalVariableValueRef("flux::Company", val))
            app.getSettings().Company = val.getString();

        if (vm.Variables.getGlobalVariableValueRef("flux::Caption", val))
            app.getSettings().Caption = val.getString();

        if (vm.Variables.getGlobalVariableValueRef("flux::Version", val))
            app.getSettings().Version = val.getString();

        if (vm.Variables.getGlobalVariableValueRef("flux::SoundPathAppend", val))
            app.getSettings().SoundPathAppend = val.getString();

        if (vm.Variables.getGlobalVariableValueRef("flux::TexturePathAppend", val))
            app.getSettings().TexturePathAppend = val.getString();

        if (vm.Variables.getGlobalVariableValueRef("flux::sdlWindowFlagsOverwrite", val))
            app.getSettings().sdlWindowFlagsOverwrite = (Uint64)val.getLong();

        if (vm.Variables.getGlobalVariableValueRef("flux::clearColor", val)) {
            SDL_Color col;
            if (MapArrayColor(val,col))
                app.getSettings().clearColor = col;
        }

    }
    void OnInit() {
        DLS::Value& funcVar = vm.Variables.getGlobal(initEventVarRegister);
        if (!vm.PlaceTapeAtScriptFunction(funcVar)) return;
        vm.run();
    }
    void OnUpdate(float dt) {
        vm.Variables.saveGlobal(var_updatedeltatime, DLS::Value(dt));

        DLS::Value& funcVar = vm.Variables.getGlobal(updateEventVarRegister);
        if (!vm.PlaceTapeAtScriptFunction(funcVar)) return;
        vm.run();
    }
    void OnShutdown() {
        DLS::Value& funcVar = vm.Variables.getGlobal(shutDownEventVarRegister);
        if (!vm.PlaceTapeAtScriptFunction(funcVar)) return;
        vm.run();
    }
    void OnRender() {
        // example:
        // OnRender = function; flux::DrawFPS 50 50 5;;

        if (renderEventVarRegister < 0) return;

        vm.Variables.saveGlobal(var_frametime, DLS::Value(BaseFlux::getFrameTime() * 100.0));

        DLS::Value& funcVar = vm.Variables.getGlobal(renderEventVarRegister);
        if (!vm.PlaceTapeAtScriptFunction(funcVar)) return;
        vm.run();

        //NOTE debug/stats:
        ArrayCount = vm.Variables.mArrays.size();
        GlobalsCount = vm.Variables.mGlobalsRegister.size();
        StackCount = vm.STACK_SIZE();
        GarbageCount = DLS::GarbageList.objects.size();
    }

    void OnKey(SDL_Keycode key, DLS::U32 EventType, SDL_Keymod mod) {
        if (keyEventVarRegister < 0) return;

        vm.Variables.saveGlobal(var_key, DLS::Value(key));
        vm.Variables.saveGlobal(var_eventType, DLS::Value(EventType));
        vm.Variables.saveGlobal(var_mod, DLS::Value(mod));

        DLS::Value& funcVar = vm.Variables.getGlobal(keyEventVarRegister);
        if (!vm.PlaceTapeAtScriptFunction(funcVar)) return;
        vm.run();
    }

    // only set the var no event
    void OnMouseMotion(const SDL_MouseMotionEvent& mmev) {
        vm.Variables.saveGlobal(var_mouseX, DLS::Value(mmev.x));
        vm.Variables.saveGlobal(var_mouseY, DLS::Value(mmev.y));
    }

    void OnMouseWheel(const SDL_MouseWheelEvent& mWheelEvent) {
        vm.Variables.saveGlobal(var_mouseWheelX, DLS::Value(mWheelEvent.x));
        vm.Variables.saveGlobal(var_mouseWheelY, DLS::Value(mWheelEvent.y));
        vm.Variables.saveGlobal(var_eventType, DLS::Value((DLS::U32)SDL_EVENT_MOUSE_WHEEL));
        if (mouseEventVarRegister < 0) return;
        DLS::Value& funcVar = vm.Variables.getGlobal(mouseEventVarRegister);
        if (!vm.PlaceTapeAtScriptFunction(funcVar)) return;
        vm.run();
    }

    void OnMouse(DLS::U8 button,  DLS::U32 EventType) {
        vm.Variables.saveGlobal(var_button, DLS::Value(button));
        vm.Variables.saveGlobal(var_eventType, DLS::Value(EventType));


        if (mouseEventVarRegister < 0) return;
        DLS::Value& funcVar = vm.Variables.getGlobal(mouseEventVarRegister);
        if (!vm.PlaceTapeAtScriptFunction(funcVar)) return;
        vm.run();
    }

    // =========================================================================
    // NOTE: Test external access to global variables:
    void printVar(std::string varName) {
        DLS::Value value;
        // if you want it fast manually do what getGlobalVariableValueRef does
        // and save the register!
        if (vm.Variables.getGlobalVariableValueRef(varName, value)) {
            DLS::Tools::printf("VAR IS: %s", value.toString().c_str());
        } else {
            DLS::Tools::errorf("Variables %s not found!", varName.c_str());
        }
    }

    // -----------------------------------------------------------------------------
    size_t getTapePos() { return TapePos;}
    size_t getArrayCount() { return ArrayCount;}
    size_t getGlobalCount() { return GlobalsCount;}
    size_t getStackCount() { return StackCount;}
    size_t getGarbageCount() { return GarbageCount;}
    //  -----------------------------------------------------------------------------

    void RegisterFluxFunctions() {
        using namespace DLS;

        FunctionMap::RegisterFunction("flux::var", [](VM&, ValueSpan args, Value& ret) -> bool {
            if (args.size() < 1) return false;
            printVar(args[0].getString());
            return true;
        }, "print a global by string varname");


        // ---------------------------------------------------------------------
        FunctionMap::RegisterFunction("flux::size", [](VM& vm, ValueSpan args, Value& ret) -> bool {
            int logiX = 0;
            int logiY = 0;

            if (args.size() >= 2) {
                SDL_Point oldSize = app.getSettings().ScreenSize;
                app.getSettings().ScreenSize.x = args[0].getInt();
                app.getSettings().ScreenSize.y = args[1].getInt();
                if (oldSize.x != app.getSettings().ScreenSize.x
                    || oldSize.y != app.getSettings().ScreenSize.y) {
                    SDL_SetWindowSize(app.getWindow(), app.getSettings().ScreenSize.x , app.getSettings().ScreenSize.y);
                    }

                    if (args.size() >= 4) {
                        logiX = args[2].getInt();
                        logiY = args[3].getInt();
                        SDL_SetRenderLogicalPresentation(app.getRenderer(), logiX, logiY ,
                                                         SDL_LOGICAL_PRESENTATION_STRETCH);
                        // SDL_LOGICAL_PRESENTATION_LETTERBOX);
                    }

            }
            SDL_RendererLogicalPresentation mode;
            SDL_GetRenderLogicalPresentation(app.getRenderer(),&logiX, &logiY, &mode);


            std::vector<Value> arr;
            arr.push_back(Value(app.getSettings().ScreenSize.x));
            arr.push_back(Value(app.getSettings().ScreenSize.y));
            arr.push_back(Value(logiX));
            arr.push_back(Value(logiY));

            ret = vm.Variables.createArray(arr, false);


            return true;
        }, "set the screen size and logical size");
        // // ---------------------------------
        FunctionMap::RegisterFunction("flux::maxfps", [](VM&, ValueSpan args, Value& ret) -> bool {
            double& l = app.getSettings().FpsLimit;
            if (args.size() == 1) {
                l = args[0].getDouble();
            }
            ret = Value(l);
            return true;
        }, "set / get the max fps");
        // // ---------------------------------
        FunctionMap::RegisterFunction("flux::Close", [](VM&, ValueSpan args, Value& ret) -> bool {
            app.TerminateApplication(); return true;
        }, "Close the Application");
        // // ---------------------------------
        // //     void DrawDebugText( SDL_Renderer *renderer,
        // //                         float x, float y, const char* text,
        // //                         float scale = 1.f, const SDL_Color& color = {100,100,100,255},
        // //                         bool doShadow = false, const SDL_Color& shadowColor = { 60,60,60,255});
        // //
        FunctionMap::RegisterFunction("flux::Text", [](VM&, ValueSpan args, Value& ret) -> bool {
            if (args.size() < 3 || !args[2].isString()) {
                Tools::errorf("flux::Text: Missing Parameter");
                return false;
            }
            float x = args[0].getFloat();
            float y = args[1].getFloat();
            const char* text = args[2].getString().c_str();

            float scale = 1.f;
            SDL_Color color = {200,200,200,255};
            bool doShadow = true;
            SDL_Color shadowcolor = {60,60,60,255};

            if (args.size() > 3)
                scale = args[3].getFloat();

            if (args.size() > 4)
                MapArrayColor(args[4], color);

            if (args.size() > 5)
                doShadow = args[5].getBool();

            if (args.size() > 6)
                MapArrayColor(args[6], shadowcolor);

            BaseFlux::DrawDebugText(app.getRenderer(),x,y,text, scale, color, doShadow, shadowcolor);

            return true;
        }, "Draw a Text on the Screen. x,y,text optional: scale,color,doShadow,shadowColor");
        //
        //
        // // ---------------------------------
        FunctionMap::RegisterFunction("flux::DrawRect", [](VM&, ValueSpan args, Value& ret) -> bool {
            if (args.size() < 1 /*|| !args[0].isArray() || !args[1].isArray()*/) {
                Tools::errorf("flux::DrawRect: Missing Parameter");
                return false;
            }
            SDL_FRect rect;
            if (!MapArrayFRect(args[0], rect)) return false;

            SDL_Color color = {255,255,255,255};
            if (args.size() > 1) MapArrayColor(args[1], color);

            bool fill = true;
            if (args.size() > 2) fill = args[2].getBool();

            BaseFlux::DrawRect(app.getRenderer(),rect, color, fill);
            return true;
        }, "Draw Rectangle Vector 4 floats, Vector 4 int, bool fill");
        // ---------------------------------
        FunctionMap::RegisterFunction("flux::fillRect", [](VM&, ValueSpan args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isPointerMarshall(SDL_FRectID)) {
                Tools::errorf("usage: SDL_RenderFillRect(Rect)\n");
                return false;
            }
            SDL_SetRenderDrawColor(app.getRenderer(),255,255,255,255);
            SDL_RenderFillRect(app.getRenderer(), static_cast<SDL_FRect*>(args[0].getPointer()));

            // SDL_FRect rect = *(static_cast<SDL_FRect*>(args[0].getPointer()));
            // SDL_RenderFillRect(app.getRenderer(), &rect);
            return true;
        }, "Draw field Rectangle Rect rect");
        // ---------------------------------
        // void DrawLine(SDL_Renderer *renderer, const SDL_FPoint& p1,const SDL_FPoint& p2 , const SDL_Color& color) {
        FunctionMap::RegisterFunction("flux::DrawLine", [](VM&, ValueSpan args, Value& ret) -> bool {
            if (args.size() < 2 || !args[0].isArray() || !args[1].isArray()) {
                Tools::errorf("flux::DrawLine: Missing Parameter");
                return false;
            }
            SDL_FPoint p1,p2;
            SDL_Color color;
            if (!MapArrayFPoint(args[0], p1)) return false;
            if (!MapArrayFPoint(args[1], p2)) return false;
            if (!MapArrayColor(args[2], color)) return false;
            BaseFlux::DrawLine(app.getRenderer(),p1, p2, color);
            return true;
        }, "Draw Line arrayOfFPoint arrayOfFPoint arrayOfColor");


        // ---------------------------------
        FunctionMap::RegisterFunction("flux::DrawFPS", [](VM&, ValueSpan args, Value& ret) -> bool {
            if (args.size() < 2) return false;

            float x = args[0].getFloat();
            float y = args[1].getFloat();

            float fontSize  = 1.0;
            if (args.size() > 2) fontSize = args[2].getFloat();


            SDL_Color color = {0,255,0,255};
            U32 fps =  BaseFlux::getFPS();

            static std::string fps_text = []{
                std::string s;
                s.reserve(32);
                return s;
            }();
            fps_text.clear();
            std::format_to(std::back_inserter(fps_text), "{} fps", fps);


            if (fps < 15) color = {255,0,0,255};
            else if (fps < 30) color = {200,50,50,255};

            BaseFlux::DrawDebugText(app.getRenderer(),x,y,fps_text.c_str(), fontSize, color, true/*, shadowColor*/);
            return true;
        }, "Draw FPS as position x y ");


        // ---------------------------------------------------------------------
        FunctionMap::RegisterFunction("flux::BatchRect", [](VM&, ValueSpan args, Value& ret) -> bool {
            if (args.size() < 3) return false;
            if (args[0].type != DLS::ValueType::Array) return false;
            if (args[1].type != DLS::ValueType::Array) return false;
            if (args[2].type != DLS::ValueType::Array) return false;

            auto rects = vm.Variables.getArrayByMasterIndex((size_t)args[0].getFastLong());
            auto colors = vm.Variables.getArrayByMasterIndex((size_t)args[1].getFastLong());
            auto filled = vm.Variables.getArrayByMasterIndex((size_t)args[2].getFastLong());

            size_t count = rects.size();
            if (colors.size() != count || filled.size() != count) return false;

            SDL_FRect sdlrect;
            SDL_Color sdlcolor;
            bool fill = false;
            for (size_t i = 0; i < count; i++) {
                if (isRect(rects[i]) && isColor(colors[i])) [[likely]] {

                    BaseFlux::DrawRect(app.getRenderer()
                        ,*(static_cast<SDL_FRect*>(rects[i].getPointer()))
                        ,*(static_cast<SDL_Color*>(colors[i].getPointer()))
                        ,filled[i].getBool());
                } else {
                    if (!MapArrayFRect(rects[i], sdlrect)) return false;
                    if (!MapArrayColor(colors[i], sdlcolor)) return false;
                    fill = filled[i].getBool();
                    BaseFlux::DrawRect(app.getRenderer(),sdlrect, sdlcolor, fill);

                }

            }

            return true;
        }, "Draw Rects params ArrayOfRects ArrayOfColors ArrayofBoolFilled ");
        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        FunctionMap::RegisterFunction("debug::arraycount", [](VM&, ValueSpan args, Value& ret) -> bool {
            size_t assigned = 0;
            for (auto& arr: vm.Variables.mArrays) {
                if (arr.isAssigned()) assigned++;
            }

            ret = Value(assigned);
            return true;
        }, "debug return assigned array count");
        FunctionMap::RegisterFunction("debug::gc", [](VM&, ValueSpan args, Value& ret) -> bool {
            vm.Variables.ArrayFreeTemp();
            size_t assigned = 0;
            for (auto& arr: vm.Variables.mArrays) {
                if (arr.isAssigned()) assigned++;
            }

            ret = Value(assigned);
            return true;
        }, "debug return assigned array count");
        // ---------------------------------------------------------------------
        auto* frectTranslator = new DLS::Marshall::GenericStructTranslator<SDL_FRect>();
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
                if (tl) tl->saveArrayValue(ret,vm.Variables.getArrayByMasterIndex((size_t)args[0].getFastLong()));
            }
            return true;
        }, "Create a Rect with marhalling.");

        // FunctionMap::RegisterFunction("Rect::destroy", [](VM&, ValueSpan args, Value& ret) -> bool {
        //     if (args.size() < 1 || !args[0].isPointerMarshall(SDL_FRectID)) return false;
        //     free(args[0].getPointer());
        //     ret = Value(0);
        //     return true;
        // }, "Destroy a Rect");


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
                if (tl) tl->saveArrayValue(ret,vm.Variables.getArrayByMasterIndex((size_t)args[0].getFastLong()));
            }
            return true;
        }, "Create a Point with marhalling");
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
                if (tl) tl->saveArrayValue(ret,vm.Variables.getArrayByMasterIndex((size_t)args[0].getFastLong()));
            }
            return true;
        }, "Create a Point3 (x,y,z) with marhalling");
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
                if (tl) tl->saveArrayValue(ret,vm.Variables.getArrayByMasterIndex((size_t)args[0].getFastLong()));
            }
            return true;
        }, "Create a Color with marhalling");
        // ---------------------------------------------------------------------
    }

    // -------------------------------------------------------------------------
    static void DLS_Logger(DLS::Tools::LogLevel level, const char* message){
        switch (level) {
            case DLS::Tools::LogLevel::warning: SDL_Log("[warn] %s",  message); break;
            case DLS::Tools::LogLevel::error:   SDL_Log("[error] %s",  message); break;
            case DLS::Tools::LogLevel::info:    SDL_Log("[info] %s",  message); break;
            default:SDL_Log("%s",  message); break;
        }
    }

    void Init (int argc, char* argv[]) {
        DLS::Init(argc,argv,DLSFlux::DLS_Logger);
        DLS::RegisterMathFunctions();
        DLS::RegisterStringFunctions( );

        RegisterFluxFunctions();
        RegisterGlobals();


        DLS::FunctionMap::RegisterFunction("testm", [](DLS::VM&, DLS::ValueSpan args, DLS::Value& ret) -> bool {
            // float foo = 47.11;
            // DLS::Marshall::ValueTranslator testTrans(&foo);
            // DLS::Tools::printf("%s", testTrans.getValue().toString().c_str());
            // testTrans.saveValue(DLS::Value(815.1));
            // DLS::Tools::printf("foo is: %f", foo);
            // ....
            const char* cstr;
            DLS::Marshall::ValueTranslator testTrans2(&cstr);
            testTrans2.saveValue(DLS::Value(47.11));
            DLS::Tools::printf("cstr is: %s", cstr);

            std::string str;
            DLS::Marshall::ValueTranslator testTrans3(&str);
            testTrans3.saveValue(DLS::Value(815));

            DLS::Value foo;
            testTrans3.loadValue(foo);
            DLS::Tools::printf("str is: %s foo is: %s", str.c_str(), foo.getString().c_str());

            return true;
        }, "test marshall");


    }


    bool Run (const std::string& command) {
        using namespace DLS;
        if (!Compile(vm,command)) return false;
        vm.run();
        TapePos = vm.tape.size();

        return true;
    }

    const std::string& getScriptFile()  { return currentFileName; }

    bool RunFile(const std::string& fileName, bool ResetTape ) {
        currentFileName = fileName;
        if (ResetTape) {
            vm.tape.clear();
            vm.pc = 0;
            vm.Variables.reset();

            ArrayCount = 0;
            GlobalsCount = 0;
            StackCount = 0;

            DLS::GarbageList.Clear();
            RegisterGlobals();
        }
        if (!Run(DLS::loadScript(fileName))) return false;
        if (ResetTape) DLSFlux::OnInit();
        return true;
    }

}
