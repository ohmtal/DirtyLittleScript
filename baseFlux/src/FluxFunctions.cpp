
#include <SDL3/SDL.h>
#include "BaseFlux/Main.h"
#include "BaseFlux/Draw.h"

#include "core/VM.h"
#include "core/FunctionMap.h"

#include "SDLTypes.h"

extern BaseFlux::Main app;
namespace DLS::Flux {

    // NOTE: Test external access to global variables:
    void printVar(VM& vm, std::string varName) {
        DLS::Value value;
        // if you want it fast manually do what getGlobalVariableValueRef does
        // and save the register!
        if (vm.Variables.getGlobalVariableValueRef(varName, value)) {
            DLS::Tools::printf("VAR IS: %s", value.toString().c_str());
        } else {
            DLS::Tools::errorf("Variables %s not found!", varName.c_str());
        }
    }


    void RegisterFluxFunctions() {

        FunctionMap::RegisterFunction("flux::var", [](VM& vm, ValueSpan args, Value& ret) -> bool {
            if (args.size() < 1) return false;
            printVar(vm,args[0].getString());
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
        FunctionMap::RegisterFunction("flux::DrawLine", [](VM& vm, ValueSpan args, Value& ret) -> bool {
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
        FunctionMap::RegisterFunction("flux::DrawFPS", [](VM& vm, ValueSpan args, Value& ret) -> bool {
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
        FunctionMap::RegisterFunction("flux::BatchRect", [](VM& vm, ValueSpan args, Value& ret) -> bool {
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
        FunctionMap::RegisterFunction("debug::arraycount", [](VM& vm, ValueSpan args, Value& ret) -> bool {
            size_t assigned = 0;
            for (auto& arr: vm.Variables.mArrays) {
                if (arr.isAssigned()) assigned++;
            }

            ret = Value(assigned);
            return true;
        }, "debug return assigned array count");

        FunctionMap::RegisterFunction("debug::gc", [](VM& vm, ValueSpan args, Value& ret) -> bool {
            vm.Variables.ArrayFreeTemp();
            size_t assigned = 0;
            for (auto& arr: vm.Variables.mArrays) {
                if (arr.isAssigned()) assigned++;
            }

            ret = Value(assigned);
            return true;
        }, "debug return assigned array count");
        // FunctionMap::RegisterFunction("Rect::destroy", [](VM&, ValueSpan args, Value& ret) -> bool {
        //     if (args.size() < 1 || !args[0].isPointerMarshall(SDL_FRectID)) return false;
        //     free(args[0].getPointer());
        //     ret = Value(0);
        //     return true;
        // }, "Destroy a Rect");
        // ---------------------------------------------------------------------
        FunctionMap::RegisterFunction("flux::SetRenderDrawColor", [](VM&,ValueSpan args, Value& ret) -> bool {
            if (args.size() < 3 ) {
                Tools::errorf("usage: flux::SetRenderDrawColor( r, g, b, a)\n");
                return false;
            }
            bool success = SDL_SetRenderDrawColor(
                app.getRenderer(),
                static_cast<Uint8>(args[0].getInt()),
                static_cast<Uint8>(args[1].getInt()),
                static_cast<Uint8>(args[2].getInt()),
                (args.size() >= 4) ?  static_cast<Uint8>(args[3].getInt()) : 255
            );
            ret = Value(success);
            return true;
        }, "flux::SetRenderDrawColor( r, g, b, a)");
        // ---------------------------------------------------------------------
        FunctionMap::RegisterFunction("flux::RenderFillRect", [](VM&,ValueSpan args, Value& ret) -> bool {
            if (args.size() != 4 ) {
                Tools::errorf("usage: flux::RenderFillRect( float x, float y, float w, float h)\n");
                return false;
            }

            SDL_FRect rect;
            rect.x = static_cast<float>(args[0].getFloat());
            rect.y = static_cast<float>(args[1].getFloat());
            rect.w = static_cast<float>(args[2].getFloat());
            rect.h = static_cast<float>(args[3].getFloat());

            ret = Value(SDL_RenderFillRect(app.getRenderer(), &rect));
            return true;
        }, "flux::RenderFillRect( float x, float y, float w, float h)");

    }



}
