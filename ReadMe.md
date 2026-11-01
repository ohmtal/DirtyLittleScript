# Dirty Little Script (DLS)

**Dirty Little Script** is a subset of [DreiZehnScript](https://github.com/ohmtal/DreiZehnScript)
it's not as performat as DreiZehn or ElfScript but it's much smaller and the base is 
header only at the moment.

## Usage in your Project: 
- Download, 
- Include the header `src/DirtyLittleScript.h`
- call `DLS::Init();`
- call `DLS::Run("print "Hello World");` 

## Cheat Sheet 

| basic | |
| --- | --- | 
| `print "hello world" 12 3.13`| no paren, space separated |
| `i=1; j=2`|semicolon only to separate multiple statement is on line|
| `print math::rand, 12`|comma only required after a nested function call|
| `if foo == 1; print "it's one"; end`|if, while, function need a `end`|
| `if foo == 1; print "it's one";;`| lazy alias for end is `;;` |
| `# this is a comment` | '#' comment. it can be used for one line| 
| `print "hello" # this is a comment` | also on the end| 
| `print "hello" # this is a comment # print "there"` | or in the middle | 
| math | |
| `f = (10 * 5 + 2 ) / 4 - 1`| basic math, respect the order|
| `b = true || false` |  boolean and or |
| `b = b + 1` |  increment one |
| Loops |  |
| `i = 5; while i > 0; print i; i = i - 1; end`| while loop | 
| `i = 5; while i > 0; print i; if i == 2; continue;; if i == 1 break;; end`| while loop | 
| Condition |  |
| `if true == false; print "Thats wrong!"; else print "OK"; end`  | if elif else |
| Function |  |
| `myFunc = function; print "Hello"; end; myFunc`  | lambda style function |
| Array |  |
| `arr[0]=10; arr[20]=100` | Numeric indexed Array auto initialized |
