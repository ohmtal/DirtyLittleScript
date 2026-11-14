//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// DLS Playground
//-----------------------------------------------------------------------------
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "imgui.h"

#include "BaseFlux/Main.h"
#include "spice/gui/ImConsole.h"
#include <format>
#include <iostream>
#include <filesystem>
#include <vector>

#include "DirtyLittleScript.h"
#include "functions/StringFunctions.h"

namespace fs = std::filesystem;

BaseFlux::Main app;
ImConsole gConsole;


std::vector<fs::path> dlsFiles;
namespace DLSFlux {
    extern void Init (int argc, char* argv[]);
    extern bool Run(const std::string& command);
    extern size_t getTapePos();
    extern size_t getArrayCount();
    extern size_t getGlobalCount();
    extern size_t getStackCount();
    extern size_t getGarbageCount();
    extern void OnSetup();
    extern void OnInit();
    extern void OnUpdate(float dt);
    extern void OnShutdown();
    extern void OnRender();
    extern bool RunFile(const std::string& fileName, bool ResetTape );
    extern void OnKey(SDL_Keycode key, DLS::U32 EventType, SDL_Keymod mod);
    extern void OnMouse( DLS::U8 button,  DLS::U32 EventType);
    extern void OnMouseMotion(const SDL_MouseMotionEvent& mmev);
    void OnMouseWheel(const SDL_MouseWheelEvent& mWheelEvent);
    extern const std::string& getScriptFile();
}

#define Log(...) SDL_Log(__VA_ARGS__)

//-----------------------------------------------------------------------------
// console redirect ....
void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    char lBuffer[1024];
    snprintf(lBuffer, sizeof(lBuffer), "%s", message);
    int wc = DLS::StrTools::getWordCount(lBuffer,'\n');
    if (wc > 1) {
        std::string str;
        for (int i = 0; i < wc; i++) {
            str = DLS::StrTools::getWord(lBuffer,i,'\n');
            if (!str.empty()) gConsole.AddLog("%s", str.c_str());
        }
    } else {
        gConsole.AddLog("%s", message);

    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void initConsole() {
    SDL_SetLogOutputFunction(ConsoleLogFunction, nullptr);

    gConsole.OnCommand = [](ImConsole* console, const char* command_line) {
        DLSFlux::Run(command_line);
    };

}
//-----------------------------------------------------------------------------

bool fetchDLSFiles() {
    std::string path = BaseFlux::Tools::getBasePath() + "assets/";

    try {
        if (fs::exists(path) && fs::is_directory(path)) {
             for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (entry.is_regular_file() && entry.path().extension() == ".dls") {
                    dlsFiles.push_back(fs::relative(entry.path(), path));

                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        SDL_Log("[error]%s",e.what());
        return false;
    }



    return true;
}

// //-----------------------------------------------------------------------------
// // Expressions
// //-----------------------------------------------------------------------------
// #include "core/Marshall.h"
// #include "core/ArenaAllocator.h"
//
// namespace DLS {
//     using namespace Marshall;
//
//     struct BoundVariable {
//         size_t registerIndex;
//         IValueTranslator* translator;
//     };
//
//     struct CompiledExpression {
//         VM vm;
//         ArenaAllocator arena;
//         std::vector<BoundVariable> bindings;
//         bool isValid = false;
//
//         void syncToVM() {
//             for (auto& b : bindings) {
//                 Value& var = vm.Variables.getGlobal(b.registerIndex);
//                 b.translator->loadValue(var);
//             }
//         }
//
//         void syncFromVM() {
//             for (auto& b : bindings) {
//                 Value& var = vm.Variables.getGlobal(b.registerIndex);
//                 b.translator->saveValue(var); // Falls deine Translator-Klasse das unterstützt
//             }
//         }
//     };
//
//     CompiledExpression CompileExpression(const VariableLinks& vars, const std::string& expr) {
//         CompiledExpression result;
//
//         for (const auto& [name, createTranslator] : vars) {
//             auto translator = createTranslator();
//             result.arena.Add(translator);
//
//             size_t reg = result.vm.Variables.reserveGlobal(SymbolTable::insert(name));
//             result.bindings.push_back({reg, translator});
//         }
//
//         result.syncToVM();
//         result.isValid = Compile(result.vm, expr);
//         return result;
//     }
//
//     Value RunExpression(CompiledExpression& compiled) {
//         if (!compiled.isValid) return Value();
//
//         compiled.syncToVM();
//         Value res = Run(compiled.vm);
//         compiled.syncFromVM();
//         return res;
//     }
//
// } // namespace DLS

#include "extras/Expressions.h"

void TestExpression() {
    using namespace DLS;
    using namespace DLS::Marshall;

    float x = 0.f;
    float y = 100.f;

    VariableLinks vars = {
        {"x", makeLink(&x)},
        {"y", makeLink(&y)}
    };

    auto expr = CompileExpression(vars, "x = math::randomf; return x+y;");

    y = 200.f;
    float res = RunExpression(expr).getFloat();
    SDL_Log("Result is: %f, new x is: %f", res, x);
}

// // void TestExpression_pre() {
// //
// //     using namespace DLS;
// //     using namespace DLS::Marshall;
// //     float x = 0.f;float y = 100.f;
// //     std::string expr = R"(
// //       print "x:" x "y:" y
// //       x = math::randomf
// //       return x+y
// //     )";
// //
// //     VariableLinks vars = {
// //         {"x", makeLink(&x)},
// //         {"y", makeLink(&y)}
// //     };
// //
// //     VM vm;
// //     //..... this must be wrapped and need a struct for that!
// //     ArenaAllocator arena; //lazy using local arena
// //     std::vector<Marshall::IValueTranslator*> translators;
// //
// //     for (const auto& [name, createTranslator] : vars) {
// //         auto translator = createTranslator();
// //         translators.push_back( translator );
// //         arena.Add(translator);
// //
// //         // 2. reserve and set variable:
// //         size_t varRegister = vm.Variables.reserveGlobal(DLS::SymbolTable::insert( name ));
// //         Value& var = vm.Variables.getGlobal(varRegister);
// //         translator->loadValue(var);
// //         SDL_Log("[info] VAR %s IS:%s", name.c_str(),var.toString().c_str());
// //     }
// //
// //     // <<<<
// //     if (!Compile(vm,expr)) return;
// //     SDL_Log("Result is: %f", Run(vm).getFloat());
// //
// //     // final goal:
// //     // CompileExpression(vm,vars, "x = math::randomf; return x+y;");
// //     // SDL_Log("Result is: %f", RunExpression(vm, vars).getFloat());
// // }
//-----------------------------------------------------------------------------
void onDraw(SDL_Renderer* renderer) {

    static bool showConsole = true;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Main")) {
            ImGui::MenuItem("Console", "GraveAccent", &showConsole);
            ImGui::MenuItem("---");
            if (ImGui::MenuItem("TestExpression")) TestExpression();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scripts")) {
            if (ImGui::MenuItem(("Reload " + DLSFlux::getScriptFile()).c_str(),"CTRL-R")) {
                DLSFlux::RunFile(DLSFlux::getScriptFile(), true);
            }

            ImGui::SeparatorText("Files");
            bool selected = false;
            for (const auto& f : dlsFiles) {
                selected = "assets/" + f.string() ==  DLSFlux::getScriptFile();
                if (ImGui::MenuItem(f.string().c_str(), nullptr, selected)) {
                    DLSFlux::RunFile("assets/" + f.string(), true);
                }

            }

            ImGui::EndMenu();
        }

        ImGui::Text("[TAPE: %06zu]", DLSFlux::getTapePos());
        ImGui::Text("[ARR: %06zu]", DLSFlux::getArrayCount());
        ImGui::Text("[GLOB: %04zu]", DLSFlux::getGlobalCount());
        ImGui::Text("[STK: %03zu]", DLSFlux::getStackCount());
        ImGui::Text("[GARB: %03zu]", DLSFlux::getGarbageCount());
        ImGui::EndMainMenuBar();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) showConsole = !showConsole;
    if (app.getImGuiIO()->KeyCtrl) {
        if (ImGui::IsKeyPressed(ImGuiKey_R))  DLSFlux::RunFile(DLSFlux::getScriptFile(), true);
    }


    gConsole.Draw("DLS Console",&showConsole);
}
//-----------------------------------------------------------------------------
void shutDown() {
    SDL_SetLogOutputFunction(nullptr, nullptr);
}
//-----------------------------------------------------------------------------
bool initApp() {



    if ( !app.InitSDL() ) return false;
    app.initImGui();



    app.OnRender = [](SDL_Renderer* renderer) {
        onDraw(renderer);
        DLSFlux::OnRender();
    };

    app.OnUpdate = [](const float deltaTime) {

        DLSFlux::OnUpdate(deltaTime);
    };

    app.OnEvent = [](const SDL_Event& event) {
        if (app.getImGuiIO())  {
            if (app.getImGuiIO()->WantTextInput) {
                if (event.type == SDL_EVENT_KEY_DOWN ||
                    event.type == SDL_EVENT_KEY_UP ||
                    event.type == SDL_EVENT_TEXT_INPUT) {
                    return;
                    }
            }
            if (app.getImGuiIO()-> WantCaptureMouse) {
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                    event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                    event.type == SDL_EVENT_MOUSE_WHEEL
                ) {
                    return;
                }

            }
        }

        // FIXME
        // ElfSDL3::onEvent(event);
        switch (event.type) {
            case SDL_EVENT_MOUSE_MOTION:
                DLSFlux::OnMouseMotion(event.motion);
                break;

            case SDL_EVENT_KEY_UP:
            case SDL_EVENT_KEY_DOWN:
            {
                DLSFlux::OnKey(event.key.key, event.type, event.key.mod);
            }
            break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                DLSFlux::OnMouse(event.button.button, event.type);
            }
            break;
            case SDL_EVENT_MOUSE_WHEEL:
            {
                DLSFlux::OnMouseWheel(event.wheel);
            }
            break;

        } //switch

    };

    app.OnShutDown = [&]() {
        DLSFlux::OnShutdown();
    };

    return true;
}
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) {

    app.getSettings() = {
        .ScreenSize = { 800, 450},
        .FpsLimit = 60,
        .EnableVSync = false,
        .Company = "BaseFlux",
        .Caption = "DLSFlux",
        .Version = "0.261105.01",
        .SoundPathAppend = "sound/",
        .TexturePathAppend = "texture/"
    };

    DLSFlux::Init(argc, argv);
    DLSFlux::RunFile("assets/main.dls", false);
    DLSFlux::OnSetup();


    if (!initApp()) return 1;

    DLSFlux::OnInit();
    fetchDLSFiles();
    initConsole();
    SDL_Log("[info] Welcome to BaseFlux DLS Console!");
    app.Execute();
    shutDown();

    return 0;
}
