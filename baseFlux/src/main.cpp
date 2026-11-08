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
    extern void Init();
    extern bool Run(const std::string& command);
    extern size_t getTapePos();
    extern size_t getArrayCount();
    extern size_t getGlobalCount();
    extern size_t getStackCount();
    extern void OnRender();
    extern bool RunFile(const std::string& fileName, bool ResetTape );
    extern void OnKey(SDL_Keycode key, DLS::U32 EventType, SDL_Keymod mod);
    extern void OnMouse( DLS::U8 button,  DLS::U32 EventType);
    extern void OnMouseMotion(const SDL_MouseMotionEvent& mmev);
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

    gConsole.OnCommand = [&](ImConsole* console, const char* command_line) {
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

//-----------------------------------------------------------------------------
void onDraw(SDL_Renderer* renderer) {

    static bool showConsole = true;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Console", "GraveAccent", &showConsole);
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

    app.OnRender = [&](SDL_Renderer* renderer) {
        onDraw(renderer);
        DLSFlux::OnRender();
    };

    // // app.OnUpdate = [&](const float deltaTime) {
    // //     const float fixedStep = 1.0f / 60.0f;
    // //     static float accumulator = 0.0f;
    // //
    // //     accumulator += deltaTime;
    // //     while (accumulator >= fixedStep) {
    // //         // // LuaState.OnUpdate(fixedStep);
    // //
    // //         accumulator -= fixedStep;
    // //     }
    // // };

    app.OnEvent = [&](const SDL_Event& event) {
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

        } //switch

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

    if (!initApp()) return 1;

    DLSFlux::Init();

    fetchDLSFiles();
    initConsole();
    SDL_Log("[info] Welcome to BaseFlux DLS Console!");
    DLSFlux::RunFile("assets/main.dls", false);
    app.Execute();
    shutDown();

    return 0;
}
