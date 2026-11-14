//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Dirty Little Script
//-----------------------------------------------------------------------------
#include <iostream>
#include <string>

#include "core/VM.h"
#include "core/Lexer.h"
#include "core/Parser.h"
#include "toolbox/Tools.h"

#include <fstream>
#include <istream>

#ifndef IMPLEMENT_DLS
#include "DirtyLittleScript.h"
#endif

namespace DLS {

    // -------------------------------------------------------------------------
    // Compile a Script
    // -------------------------------------------------------------------------
    bool Compile(VM& vm, const std::string& code) {
        Lexer lex(code);
        TokenVector token = lex.tokenize();
        if (DEBUG_TOKEN) {
            for (auto& tok: token) {
                std::cout << tokenTypeToString(tok.mType) << "  ::  " << tok.mValue << "\n";
            }
        }

        Parser parser(token,&vm);
        if (!parser.parse() ) {
            return false;
        }
        if (vm.tape.size() == 0) return false;
        return true;
    }
    // -------------------------------------------------------------------------
    // Run the tape return a copy of the last stack value
    // -------------------------------------------------------------------------
    const Value Run( VM& vm, bool doRewind ) {
        Value result = Value("");
        if (vm.tape.size() == 0) {
            return result;
        }
        if (doRewind) vm.rewind();
        vm.run();
        U32 stackSize =  vm.STACK_SIZE();
        if ( stackSize > 0) {
            result = vm.stack[stackSize - 1];
        }
        return result;
    }

    // -------------------------------------------------------------------------
    // Run a Script, and shutdown if you need to keep it up vm must be defined
    //               in your source so you can work with it.
    // -------------------------------------------------------------------------
    std::string Run(const std::string& code) {
        VM vm;
        if (!Compile(vm, code)) return "";
        vm.run();
        std::string result = "";
        for (size_t iter = 0; iter < vm.STACK_SIZE(); iter++)
        {
            if (iter > 0) result = result  + ", ";
            result = result + vm.stack[iter].toString();
        }
        return result;
    }
    // -------------------------------------------------------------------------
    // Load a Script file
    // -------------------------------------------------------------------------
   std::string loadScript(const std::string& scriptPath) {
        std::ifstream file(scriptPath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            Tools::errorf("Script file cant be opened: %s\n", scriptPath.c_str());
            return "";
        }
        return std::string((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    }
    // -------------------------------------------------------------------------
    // Init the base functions and constants
    // -------------------------------------------------------------------------

    void Init() {
        if (_init_done_) return ; _init_done_ = true;

        using namespace FunctionMap;

#ifndef DLS_NO_EXTRA_FUNCTIONS
        #ifndef DLS_NO_MATH
            RegisterMathFunctions();
        #endif
        #ifndef DLS_NO_STRINGS
            RegisterStringFunctions();
        #endif
        #ifndef DLS_NO_HELP
            RegisterHelpFunctions();
        #endif
#endif
        // ---------------------------------------------------------------------
        // also push some CORE Constants here:
        RegisterConstants("true", Value(1));
        RegisterConstants("false", Value(0));

        // ---------------------------------------------------------------------

        RegisterConstants("type::float",    (int32_t)ValueType::Double);
        RegisterConstants("type::integer",  (int32_t)ValueType::Long);
        RegisterConstants("type::pointer",  (int32_t)ValueType::Pointer);
        RegisterConstants("type::array",    (int32_t)ValueType::Array);
        RegisterConstants("type::string",   (int32_t)ValueType::String);
        RegisterConstants("type::function", (int32_t)ValueType::Function);

        RegisterFunction("getType", [](ValueSpan args, Value& ret ) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: getType value. return the type id of the expression. (Example: Type::float)");
                ret =  Value(-1);
                return true;
            }
            ret = Value((int) args[0].type);
            return true;
        } ,"Usage: getType value. return the type id of the expression. (Example: Type::float)");



        // ---------------------------------------------------------------------
        // -------- print --------------
        // ---------------------------------------------------------------------
        RegisterFunction("print", [](ValueSpan args, Value& ret) -> bool {
            std::string line = "";
            for ( auto& value : args) {
                line = line + value.toString() + " ";
            }
            Tools::printf("%s\n", line.c_str());
            return true;
        }, "Print a line to console. @param value value value ...");

        RegisterFunction("printraw", [](ValueSpan args, Value& ret) -> bool {
            std::string line = "";
            for ( auto& value : args) {
                line = line + value.toString();
            }
            Tools::printf("%s", line.c_str());
            return true;
        }, "Print a line without linefeed to console. @param value value value ...");

        RegisterFunction("error", [](ValueSpan args, Value& ret) -> bool {
            std::string line = "";
            for ( auto& value : args) {
                line = line + value.toString() + " ";
            }
            Tools::errorf("%s\n", line.c_str());
            return true;
        }, "Print a error to console. @param value value value ...");
        // ---------------------------------------------------------------------
        // int
        // ---------------------------------------------------------------------
        RegisterFunction("int::cast", [](ValueSpan args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: int::cast value\n");
                return false;
            }
            if (args[0].isInt()) ret =  args[0];
            else
                if (args[0].isDouble()) ret = Value(args[0].getInt());
                else
                if (args[0].isString()) ret = Value( std::atoi( args[0].getString().c_str()));
                else
                Tools::errorf("Cant cast to integer!\n");

            return true;
        }, "cast any type to integer.");


        // ---------------------------------------------------------------------
        // float
        // ---------------------------------------------------------------------
        RegisterFunction("float::cast", [](ValueSpan args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: float::cast value\n");
                return false;
            }
            if (args[0].isDouble()) ret =  args[0];
            else
                if (args[0].isInt()) ret = Value(args[0].getDouble());
                else
                if (args[0].isString()) ret = Value( std::atof( args[0].getString().c_str()));
                else
                Tools::errorf("Cant cast to float!\n");

            return true;
        }, "cast any type to float");
        // ---------------------------------------------------------------------
        // Array
        // ---------------------------------------------------------------------
        RegisterFunction("array::count", [](ValueSpan args, Value& ret) -> bool {
            if (args.size() != 1 || args[0].type != ValueType::Array) {
                Tools::errorf("Usage: array::count value\n");
                return false;
            }
            if (!Globals::CurrentVM) {
                Tools::errorf("Sorry no VM running.\n");
                return false;
            }

            ValueVector arr;
            if (!Globals::CurrentVM->Variables.getArrayByMasterIndexSafe((size_t)args[0].getLong(), arr)) {
                Tools::errorf("Array Index out of bounds.\n");
                return false;
            }

            ret = Value((S64)arr.size());
            return true;
        }, "return the element count of an array variable");

    } //<< Init
    // -------------------------------------------------------------------------
    // Full init
    // -------------------------------------------------------------------------

    std::vector<std::string> CmdlineArgs;

    void Init(int argc , char* argv[], Tools::LogCallback customLogHandler ) {
        static bool registered = false; if (registered) return; registered = true;
        using namespace FunctionMap;

        if (customLogHandler) Tools::LogHandler = customLogHandler;

        // see also sys::argv function
        if (argv) {
            RegisterConstants("sys::argc",Value(argc));
            for (int i = 0; i < argc; i++) {
                CmdlineArgs.push_back(std::string(argv[i]));
            }
        }

        Init(); // init the VM
        // ---------------------------------------------------------------------
        // Argv
        // ---------------------------------------------------------------------
        RegisterFunction("sys::argv", [](ValueSpan args, Value& ret) -> bool {
            if (args.size() != 1) {
                Tools::errorf("Usage: sys::argv index\n");
                return false;
            }
            size_t index = (size_t)args[0].getLong();
            if (index >= CmdlineArgs.size()) return false;
            ret = Value(CmdlineArgs[index]);
            return true;
        }, "get a command line argument by index. see also const sys::argc (count)");

    }

} // namespace DLS
