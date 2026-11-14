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

#include <thread>

#include "DirtyLittleScript.h"
#include "functions/StringFunctions.h"

namespace fs = std::filesystem;

BaseFlux::Main app;
ImConsole gConsole;


std::vector<fs::path> dlsFiles;
namespace DLS::Flux {
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
        DLS::Flux::Run(command_line);
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

// =============================================================================
#include "extras/Expressions.h"



void TestExpression(int tnum) {
    SDL_Log("running %d....\n", tnum);

    using namespace DLS;
    using namespace DLS::Marshall;

    float x = 0.f;
    double y = 100.f;

    VariableLinks vars = {
        {"tnum", makeLink(&tnum)},
        {"x", makeLink(&x)},
        {"y", makeLink(&y)}
    };

    auto expr = CompileExpression(vars, "x = math::randomf; y = y + x; return tnum");

    y = 0.f;
    int res = 0;
    for (size_t i = 0; i < 1000000; i++) {
        res = RunExpression(expr).getInt();
        if (i % 1000 == 0) {
            SDL_Delay(1);
        }
    }
    SDL_Log("#%02d: x is: %f, y is: %f\n", res, x, y);
}

void testThread() {

    const int numThreads = 16;
    std::vector<std::thread> threads;
    SDL_Log("------- starting %d threads -------\n", numThreads);

    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([i] {
            TestExpression(i);
        });
    }

    for (auto& t : threads) {
        // t.join();
        t.detach();
    }

    // SDL_Log("All VM threads completed.");

    // std::thread t([] {
    //    TestExpression();
    // });
    // t.detach();
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
            if (ImGui::MenuItem("TestThread with Expression")) testThread();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scripts")) {
            if (ImGui::MenuItem(("Reload " + DLS::Flux::getScriptFile()).c_str(),"CTRL-R")) {
                DLS::Flux::RunFile(DLS::Flux::getScriptFile(), true);
            }

            ImGui::SeparatorText("Files");
            bool selected = false;
            for (const auto& f : dlsFiles) {
                selected = "assets/" + f.string() ==  DLS::Flux::getScriptFile();
                if (ImGui::MenuItem(f.string().c_str(), nullptr, selected)) {
                    DLS::Flux::RunFile("assets/" + f.string(), true);
                }

            }

            ImGui::EndMenu();
        }

        ImGui::Text("[TAPE: %06zu]", DLS::Flux::getTapePos());
        ImGui::Text("[ARR: %06zu]", DLS::Flux::getArrayCount());
        ImGui::Text("[GLOB: %04zu]", DLS::Flux::getGlobalCount());
        ImGui::Text("[STK: %03zu]", DLS::Flux::getStackCount());
        ImGui::Text("[GARB: %03zu]", DLS::Flux::getGarbageCount());
        ImGui::EndMainMenuBar();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent)) showConsole = !showConsole;
    if (app.getImGuiIO()->KeyCtrl) {
        if (ImGui::IsKeyPressed(ImGuiKey_R))  DLS::Flux::RunFile(DLS::Flux::getScriptFile(), true);
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
        DLS::Flux::OnRender();
    };

    app.OnUpdate = [](const float deltaTime) {

        DLS::Flux::OnUpdate(deltaTime);
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
                DLS::Flux::OnMouseMotion(event.motion);
                break;

            case SDL_EVENT_KEY_UP:
            case SDL_EVENT_KEY_DOWN:
            {
                DLS::Flux::OnKey(event.key.key, event.type, event.key.mod);
            }
            break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                DLS::Flux::OnMouse(event.button.button, event.type);
            }
            break;
            case SDL_EVENT_MOUSE_WHEEL:
            {
                DLS::Flux::OnMouseWheel(event.wheel);
            }
            break;

        } //switch

    };

    app.OnShutDown = [&]() {
        DLS::Flux::OnShutdown();
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
        .Caption = "DLS::Flux",
        .Version = "0.261105.01",
        .SoundPathAppend = "sound/",
        .TexturePathAppend = "texture/"
    };

    DLS::Flux::Init(argc, argv);
    DLS::Flux::RunFile("assets/main.dls", false);
    DLS::Flux::OnSetup();


    if (!initApp()) return 1;

    DLS::Flux::OnInit();
    fetchDLSFiles();
    initConsole();
    SDL_Log("[info] Welcome to BaseFlux DLS Console!");
    app.Execute();
    shutDown();

    return 0;
}
