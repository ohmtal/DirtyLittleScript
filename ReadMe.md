# Dirty Little Script (DLS)

**Dirty Little Script** is a **header only** subset of [DreiZehnScript](https://github.com/ohmtal/DreiZehnScript)


## Keynotes 

- can be used as a Expression compiler / runner
- only global variables
- no objects (fields/methods) like in DreiZehn but basic structs support 
- only lambda style functions without parameters
- array: only number indexed and auto initialize on current dimension


## Usage in your Project

### header only :: faster implementation to your project:
- Download and add `DirtyLittleScript/dls` to your include path 
- `#define IMPLEMENT_DLS` *only once* and include the header `DirtyLittleScript.h` 
- call `DLS::Init();`
- call `DLS::Run("print "Hello World");` 
- if you dont need math and string extra functions: `#define DLS_NO_EXTRA_FUNCTIONS` .This make compiling faster. 


## Using as Lib :: faster compile time
- Download `DirtyLittleScript` 
- in cmake `include ("../dls/DLS_lib.cmake")` replace `../` with the path where you installed it
- add `DirtyLittleScript` to your target_link_libraries


## Cheat Sheet 

| basic | |
| --- | --- | 
| `print "hello world" 12 3.13`| no paren, space separated |
| `i=1; j=2`|semicolon only to separate multiple statement is on line|
| `print (math::rand) 12`|encapsulate your nested function calls with paren|
| `if foo == 1; print "it's one"; end`|if, while, function need a `end`|
| `if foo == 1; print "it's one";;`| lazy alias for end is `;;` |
| `# this is a comment` | '#' comment. it can be used for one line| 
| `print "hello" # this is a comment` | also on the end| 
| `print "hello" \ ignored` | to break a line use `\`. everything behind is ignored (*5)| 
| `      "World" ` | but on the next line the code continue.| 
| math | |
| `f = (10 * 5 + 2 ) / 4 - 1`| basic math, the parser respect the order|
| `b = true && false` |  boolean and, or |
| `b = b + 1` |  increment one |
| `b++; b--` |  increment and decrement again (only on variables)|
| Loops |  |
| `forRange i 10; print i;  end`| forRange loop count i 0..9 | 
| `i = 5; while i > 0; print i; i = i - 1; end`| while loop | 
| `i = 5; while i > 0; print i; if i == 2; continue;; if i == 1 break;; end`| while loop | 
| Condition |  |
| `if true == false; print "Thats wrong!"; else print "OK"; end`  | if elif else |
| Function |  |
| `myFunc = function; print "Hello"; end; myFunc`  | lambda style function (*1)  |
| Array |  |
| `arr[0]=10; arr[20]=100` | Numeric indexed Array auto initialized (*2)|
| `arr = { 10 20 30}` | construct a array with three elements |
| `arr[0][0] = 10` | multi dimension array. must be initialized before! like `arr[0]={10 20 30}` (*4) |
| Objects | Rect Example from BaseFlux Demo(*3) |
| `r = Rect::new {10 10 30 30}` | Create an Rect and push it into Garbage Collections|
| `r.x = 10` | set x to 10 |
| `print r.x` | Read X from the Rect |
| Help | like in DreiZehnScript i added a help system |
| `help::syntax` | print syntax informations. | 
| `help::fn`     | print the available Function | 
| `help::const`  | print the registered Constants | 
| `help::glob`   | print all registered Global Variables | 
| Example Scripts | |
| [Hello World](https://github.com/ohmtal/DirtyLittleScript/blob/main/demo/scripts/helloworld.dls)| |
| [FizzBuzz](https://github.com/ohmtal/DirtyLittleScript/blob/main/demo/scripts/fizzbuzz.dls)| |
| [99 Bottles](https://github.com/ohmtal/DirtyLittleScript/blob/main/demo/scripts/99Bottles.dls)| |
| [Starfield](https://github.com/ohmtal/DirtyLittleScript/blob/main/sdldemo/scripts/StarField.dls)| |


<small>(*1) only in DLS and not in DreiZehnScript available</small>

<small>(*2) DreiZehnScript handle arrays with objects so it's different. 
I set a max elements limit to 65536. Which can be changed with compile flag: DLS_MAX_ARRAY_ELEMENTS</small>

<small>(*3) DreiZehnScript use a system to modify values directly by Pointer which is faster but disable Marshalling. So this Pointer to Struct System is only in DLS.</small>

<small>(*4) Using this is slower than using multile single dimension arrays</small>

<small>(*5) Different to DreiZehn: the code behind `\` is not ignored in DreiZehnScript.</small>

## Bind your C-Functions

This is simular to [DreiZehn's binding method](https://github.com/ohmtal/DreiZehnScript/blob/main/handbook/Bindings.md).
Except the Object. DLS does not support the DreiZehn ValueObject Classes.


## Compile the demo

*Note: you need a c++ compiler and cmake installed*

```
cd demo
cmake -S . -B build
cmake --build build
```

## Technology 

- require C++ 20
- Lexer: Traditional Tokenizer
- Value: 16 Byte typed with union struct
- Parser: Pratt (Operator-precedence parser) writing directly to the tape without a AST
- VM: ByteCode Tape with 32bit slots. ~~threaded flatten tape - a bit like bytecode~~
- Functionmap - C-function system from DreiZehn 
- Marshalling for basic structs with field access. 


## Compile Flags

| Flag (#define) | Comment |
| --- | --- | 
| IMPLEMENT_DLS | When used header only: This must be added once before `DirtyLittleScript.h`|
| DLS_NO_EXTRA_FUNCTIONS | Disable all extra functions: math, strings, help,...|
| DLS_NO_MATH | Disable math functions|
| DLS_NO_STRINGS | Disable string functions|
| DLS_NO_HELP | Disable help functions|
|  |  |

## Using as Expression compiler

```
//-----------------------------------------------------------------------------
// DLS Expression Demo
//-----------------------------------------------------------------------------
// when used as header only: #define IMPLEMENT_DLS
#include "DirtyLittleScript.h"
// must be included because it's not part of the core
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

    CompiledExpression expr = CompileExpression(vars, "x = math::random 0 y; return x");

    if (!expr.isValid) {
        printf("\n[ERROR] Compile failed!\n");
        return;
    }

    y = 1.f;
    float res = 0.f;

    for (int i = 0; i < 100; i++) {
        y = 1.f +(float) (i * 10) ;
        res = RunExpression(expr).getFloat();
        printf("Result is: %f, x is: %f, y is: %f\n", res, x, y);
    }
}


int main(int argc, char* argv[]) {
    DLS::Init();
    TestExpression();
}

```
