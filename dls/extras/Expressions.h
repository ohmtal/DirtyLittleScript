//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Expressions Spezial to use it as an Expression evaluator.
//-----------------------------------------------------------------------------
// Example:
//     #include "extras/Expressions.h"
//     void TestExpression() {
//         using namespace DLS;
//         using namespace DLS::Marshall;
//
//         float x = 0.f;
//         float y = 100.f;
//
//         VariableLinks vars = {
//             {"x", makeLink(&x)},
//             {"y", makeLink(&y)}
//         };
//
//         auto expr = CompileExpression(vars, "x = math::randomf; return x+y;");
//
//         y = 200.f;
//         float res = RunExpression(expr).getFloat();
//         printf("Result is: %f, new x is: %f", res, x);
//     }
//-----------------------------------------------------------------------------
#include "core/Marshall.h"
#include "core/ArenaAllocator.h"

namespace DLS {
    using namespace Marshall;

    struct BoundVariable {
        size_t registerIndex;
        IValueTranslator* translator;
    };

    struct CompiledExpression {
        VM vm;
        ArenaAllocator arena;
        std::vector<BoundVariable> bindings;
        bool isValid = false;

        void syncToVM() {
            for (auto& b : bindings) {
                Value& var = vm.Variables.getGlobal(b.registerIndex);
                b.translator->loadValue(var);
            }
        }

        void syncFromVM() {
            for (auto& b : bindings) {
                Value& var = vm.Variables.getGlobal(b.registerIndex);
                b.translator->saveValue(var); // Falls deine Translator-Klasse das unterstützt
            }
        }
    };

    inline CompiledExpression CompileExpression(const VariableLinks& vars, const std::string& expr) {
        CompiledExpression result;

        for (const auto& [name, createTranslator] : vars) {
            auto translator = createTranslator();
            result.arena.Add(translator);

            size_t reg = result.vm.Variables.reserveGlobal(SymbolTable::insert(name));
            result.bindings.push_back({reg, translator});
        }

        result.syncToVM();
        result.isValid = Compile(result.vm, expr);
        return result;
    }

   inline  Value RunExpression(CompiledExpression& compiled) {
        if (!compiled.isValid) return Value();
        compiled.syncToVM();
        Value res = Run(compiled.vm, true);
        compiled.syncFromVM();
        return res;
    }

} // namespace DLS
