# Dirty Little Script (DLS)

**Dirty Little Script** is a **header only** subset of [DreiZehnScript](https://github.com/ohmtal/DreiZehnScript)

## Usage in your Project: 
- Download and add `DirtyLittleScript/dls` to your include path 
- Include the header `DirtyLittleScript.h`and `#define IMPLEMENT_DLS` *only once*
- call `DLS::Init();`
- call `DLS::Run("print "Hello World");` 
- optional add dls/functions/*.cpp to you project. (see also demo)

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
| math | |
| `f = (10 * 5 + 2 ) / 4 - 1`| basic math, the parser respect the order|
| `b = true && false` |  boolean and, or |
| `b = b + 1` |  increment one |
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
| `arr[0][0] = 10` | multi dimension array. must be initialized before! like `arr[0]={10 20 30}` |

(*1) only in DLS and not in DreiZehnScript available

(*2) DreiZehnScript handle arrays with objects so it's different


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
- Parser: Pratt (Operator-precedence parser) writing directly to the tape
- VM: threaded flatten tape - a bit like bytecode but with (bigger) slots 
- Functionmap - C-function system from DreiZehn 
