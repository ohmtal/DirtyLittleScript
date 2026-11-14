//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Core + Help Commands + sys commands
//-----------------------------------------------------------------------------
#include <string>
#include <algorithm>
#include "core/FunctionMap.h"
#include "core/Globals.h"
#include "core/VM.h"
#include "toolbox/Tools.h"

namespace DLS {

    // =============================================================================
    // --- RegisterCoreFunctions ---
    // =============================================================================
    void RegisterHelpFunctions( ) {
        static bool registered = false; if (registered) return; registered = true;
        using namespace FunctionMap;





        // ---------------------------------------------------------------------
        // Help
        // ---------------------------------------------------------------------
        RegisterFunction("help", [](VM&, ValueSpan args, Value& ret) -> bool {
            Tools::printf("---------------- DLS Help ------------------\n");
            Tools::printf(
                "help::syntax    => print syntax informations.\n"
                "help::fn        => print the available Function\n"
                "help::const     => print the registered Constants\n"
                "help::glob      => print all registered Global Variables\n"
            );
            return true;
        }, "print the help overview");
        RegisterFunction("help::syntax", [](VM&, ValueSpan args, Value& ret) -> bool {
            Tools::printf(
                "Basics\n"
                "- parameters for functions can be simple written word by word.\n"
                "    Exception: when you call a method you have to add parantheses\n"
                "    Example: example `print (myobj->getX) (myobj->getX)`\n"
                "- Static functions like Array::new are separated by '::'\n"
                "- Tool functions first character should by lowercase: core::gc\n"
                "- Objects first character should by uppercase: Array::new\n"
                "- Object fields can be accessed by Dot '.' Separator: myVector.x = 3\n"
                "- Object methods can be called with the Arrow '->' like Array->print\n"
                "- optional ';' is a line break for the code so it's the same as your write it in a new line.\n"
                "Syntax examples:\n"
                "Operators: '+' '-' '*' '/' '|' '||' '&' '&&' '>>' '<<'\n"
                "Operators: '>' '<' '==' '!='\n"
                "Inline Operators: '++' '--'\n"
                "Assigment: '='\n"
                " ------------- \n"
                "Iterators:\n"
                "forRange i 10; print i; end => print i from 0 to 9\n"
                "i = 0;while i < 10; print i; i++; end => print i from 0 to 9\n"
                "Note: `break` break the current loop.\n"
                " ------------- \n"
                "Condition:\n"
                "i = 4; if i == 4; print \"It's four\"; else  print \"It's not four\";end\n"
                " ------------- \n"
                "Function Declaration\n"
                "myFunction = function; print \"Hello\";;\n"
                "\n"

            ) ;
            return true;
        }, "basic Syntax informations");


        // ---------------------------------------------------------------------
        RegisterFunction("help::fn", [](VM&, ValueSpan args, Value& ret) -> bool {
            bool doFilter = false;
            std::string filter;
            if (!args.empty() && args[0].isString()) {
                filter = args[0].getString();
                if (!filter.empty()) doFilter = true;
            }

            struct NamedFunction {
                uint32_t key;
                std::string name;
            };
            std::vector<NamedFunction> filteredFunctions;

            for (const auto& [key, value] : RegisteredFunctions) {
                std::string name = SymbolTable::getName(key);
                if (!doFilter || name.find(filter) != std::string::npos) {
                    filteredFunctions.push_back({key,name});
                }
            }

            // std::vector<std::string> scriptNames;
            // for (const auto& [key, value] : RegisteredScriptFunctions) {
            //     std::string name = SymbolTable::getName(key);
            //     if (!doFilter || name.find(filter) != std::string::npos) {
            //         scriptNames.push_back(name);
            //     }
            // }

            std::sort(filteredFunctions.begin(), filteredFunctions.end(),
                      [](const NamedFunction& a, const NamedFunction& b) {
                          return a.name < b.name;
                      }
            );

            // std::sort(scriptNames.begin(), scriptNames.end());

            Tools::printf("  --- Functions [%zu/%zu] --- \n", filteredFunctions.size(), RegisteredFunctions.size());
            for (const auto& func : filteredFunctions) {
                uint32_t strId =  getFunctionDescStrId(func.key);
                const std::string desc = (strId > 0 ) ? StringTable::get(strId) : "";
                Tools::printf("  - %-30s %s\n", func.name.c_str(), desc.c_str());
            }

            // if (scriptNames.size() > 0) {
            //     Tools::printf("  --- Script Functions [%zu/%zu] --- \n", scriptNames.size(), RegisteredScriptFunctions.size());
            //     for (const std::string& name : scriptNames) {
            //         Tools::printf("  - %s \n", name.c_str());
            //     }
            // }

            return true;
        }, "list the registered functions. you can call it with a filter like `help::fn \"str\"`");

        // ---------------------------------------------------------------------
        RegisterFunction("help::const", [](VM&, ValueSpan args, Value& ret) -> bool {
            bool doFilter = false;
            std::string filter;
            if (!args.empty() && args[0].isString()) {
                filter = args[0].getString();
                if (!filter.empty()) doFilter = true;
            }


            std::vector<std::string> constNames;
            for (const auto& [key, value] : RegisteredConstants) {
                std::string name = SymbolTable::getName(key);
                if (!doFilter || name.find(filter) != std::string::npos) {
                    constNames.push_back(name);
                }
            }


            std::sort(constNames.begin(), constNames.end());

            Tools::printf("  --- Constants [%zu/%zu] --- \n", constNames.size(), RegisteredConstants.size());
            for (const std::string& name : constNames) {
                // lazy recover
                const uint32_t id = SymbolTable::insert(name);
                Value value = RegisteredConstants[id];
                Tools::printf(" %20s = %s\n", name.c_str(), value.toString().c_str());
            }



            return true;
        }, "list all registerd constants can be called with a filter.");

        // ---------------------------------------------------------------------
        RegisterFunction("help::glob", [](VM& vm, ValueSpan args, Value& ret) -> bool {
            Tools::printf("---------------- Globals ------------------\n");
            for (auto & globs: vm.Variables.mGlobalsLookUp) {
                Value& val = vm.Variables.getGlobal(globs.second);
                Tools::printf("%s = %s\n "
                ,  SymbolTable::getName(globs.first).c_str()
                , val.toString().c_str()
                );
            }
            Tools::printf("------------------------------------------------------\n");
            return true;
        }, "Print all global variables.");
        // ---------------------------------------------------------------------
    } //RegisterCoreFunctions

} //namespace
