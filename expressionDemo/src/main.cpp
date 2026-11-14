//-----------------------------------------------------------------------------
// DLS Expression Demo
//-----------------------------------------------------------------------------
// when used as header only: #define IMPLEMENT_DLS
#include "DirtyLittleScript.h"
// must be included because it's not part of the core
#include "extras/Expressions.h"

// we need this namespaces:
using namespace DLS;
using namespace DLS::Marshall;

void TestExpression() {
    // setup some variables
    float x = 0.f;
    float y = 100.f;

    // setup Variable links
    VariableLinks vars = {
        {"x", makeLink(&x)},
        {"y", makeLink(&y)}
    };

    // Compile the Expression once
    CompiledExpression expr = CompileExpression(vars, "x = math::random 0 y; return x");

    // check it's not failed
    if (!expr.isValid) {
        printf("\n[ERROR] Compile failed!\n");
        return;
    }

    // do something with it:
    y = 1.f;
    float res = 0.f;

    // we run the compiled Expression 100 times
    for (int i = 0; i < 100; i++) {
        y = 1.f +(float) (i * 10) ;
        res = RunExpression(expr).getFloat();
        printf("Result is: %f, x is: %f, y is: %f\n", res, x, y);
    }
}

void TestEnhancedExpression () {
    // setup some variables
    float x = 0.f;
    float y = 100.f;

    // setup Variable links
    VariableLinks vars = {
        {"x", makeLink(&x)},
        {"y", makeLink(&y)}
    };

    std::string expression = R"(
        print "~~~ More than just math ;) ~~~"
        if math::random 0 100 >= 50
            x = 47.11
            y = 8.15
        else
            x = 8.15
            y = 47.11
        end
        return "Hello World"
    )";

    // Compile the Expression once
    CompiledExpression expr = CompileExpression(vars, expression);

    // check it's not failed
    if (!expr.isValid) {
        printf("\n[ERROR] Compile failed!\n");
        return;
    }

    // we run the compiled Expression
    std::string result = "";
    result = RunExpression(expr).toString();
    printf("Result is: %s, x is: %f, y is: %f\n", result.c_str(), x, y);
}


int main(int argc, char* argv[]) {
    DLS::Init(); // Once to setup DLS
    TestExpression();
    TestEnhancedExpression();
}
