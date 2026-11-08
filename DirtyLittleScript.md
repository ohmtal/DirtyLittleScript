# Dirty Little Script (DLS)

i take a look on a different attempt:  using Flatten Tape

*Note on Highlighting:* DLS does not have `--` for comments only `#` so here I use
**TCSH** highlighter. 

This time it should be really small.

- Lexer: Traditional Tokenizer
- Value: 16 Byte typed with union 
- Parser: Pratt 
- VM: Flatten Tape
- Functionmap from DreiZehn (SymbolTable to function map)

## Features:
- Identifer
    - [X] c-function
    - [X] const
    - [X] var (only globals!)
- [X] Literals int/string
- [X] Math `+` `-` `*` `/` 
- [X] Compare `==` `!=` `<` `>` `<=` `>=`
- [X] C-Function Calls 
    - [X] nested with , and ()
    - [X] reverse order POP_STACK)
- [X] If, Elif, else
- [X] Define 
- [X] While 
- Assign `=` 
    - [X] globals

### Time run test_var.dls the first time (2026-10-31)

- using while 
- using math like `a = a + 1` since i dont have inline assign like ++ or *=
- Release with debug

```
 Performance counter stats for './dlsDemo scripts/test_var.dls':

    41,670,890,908      cycles:u                                                              
    80,278,726,253      instructions:u                                                        
    12,138,309,314      branches:u                                                            
        28,661,639      branch-misses:u                                                       
         1,434,520      L1-icache-load-misses:u                                               

      13.562182971 seconds time elapsed

      13.506357000 seconds user
       0.003825000 seconds sys

```

### Same Script with AST-Interpreter DreiZehn

```
 Performance counter stats for './DreiZehn dls/scripts/test_var.dls':

    18,740,336,533      cycles:u                                                              
    52,433,298,431      instructions:u                                                        
    12,751,720,010      branches:u                                                            
         6,225,244      branch-misses:u                                                       
           475,395      L1-icache-load-misses:u                                               

       6.115660131 seconds time elapsed

       6.075584000 seconds user
       0.003937000 seconds sys
```

### Same with ByteCode DreiZehn:

```
 Performance counter stats for './DreiZehn -bc dls/scripts/test_var.dls':

    11,001,946,125      cycles:u                                                              
    33,558,293,791      instructions:u                                                        
     4,289,213,272      branches:u                                                            
        14,701,883      branch-misses:u                                                       
           635,983      L1-icache-load-misses:u                                               

       3.598581406 seconds time elapsed

       3.575701000 seconds user
       0.000997000 seconds sys
       
```
    
So DreiZehn is much faster - i spend a lot of time to optimize it ;) The while 
loop is simular to what i do in DreiZehn bytecode, so I'am  relative sure it's 
the DreiZehn  Variable Pointer Cache and only 8 byte for value instead of 16.
For first run ever 13,50 it's not so bad - still beat python on that ;) 

## Features continue:

- [X] basic array 
- [X] And / Or 
- [X] bit: and, or, xor, shl, shr
- [X] fn lamda like without name , parameters or return value *haha*
- Done and ready for cleanup push? << NOT!

## SDL Test Starfield:
### Bugs
- [X] CRIT: Array is not working !!!! POINT_X[1] = 1; POINT_Y[1] = 2; POINT_X[1] becomes 2?!
- [X] fn NOT working! Test with function calls << it is! it did not find the variables!!!
- [X] inside "#" disable the code ! << tested works ?! 

### Todos:
- [X] write readme 
- [X] write examples
- [X] write cheat sheet 
- [X] rewrite SymbolTable/map lookup to register based vector for the global variables
    - [X] variables ==> double speed on test_var ! 6.690010350 seconds time elapsed
    
- [X] i broke the parser someewhere - since variable registers? - test does not pass! 
    - **NOTE** when i run it single it pass! There must be chunk left from previous run!
    - reset all global vectors build in VM !
        - [X] VariableStorage - a new one every time << did not fix it!
        - [X] It's the contant `define foo` this breaks the following !!!
              NEVER use lowercase with define !! :P ***it's not a bug***
    - cause assert stack overflow: `mySrc = R"( if; 1 < 2; 40; end; )"; std::cout << mySrc << "  =  " << Run(mySrc) << "\n";`
    - unhandled token LSquare =>  `mySrc = R"( foo[110] = 10; print "10==" foo[110] )"; Run(mySrc);`
    - Unhandled Token: Assign => ` foo = function ; print str::strlen "HUHU";;`
    - Unhandled Token: Assign => somewhere in:  *Comment test*
    
- [X] add parse error  constansPointer ==> peek Assign or '[' or move the constant to bottom   

- [X] remove define again! was a bad hack fest anyway 

- [X] cache c-function pointer on tape.<< NOT FASTER!

- [X] Critical: create a endless loop!!!! `fn = function; print "hello World";;`
    also only `fn` create a endless loop


- [X] on assing array, it  must be copied .. bad grow if used a lot .. 

- --------------- TAGGED 0.3 ---------------

- [X] Syntax is done remove unused tokens
    
- [X] funny: `i = 0; while i < 100; i++;;` gived an unhandled "end" error
because `i++` is invalid ?! fixed after removing unused token! 

- [X] changed array assign copy on save global 

- Features completed ?! 
        
- --------------- TAGGED 0.4 ---------------

- [X] Added forRange
- [X] Added curly constuctor        
- [X] Bug in Array setAssigned

This works fine:
```
InitRects = function
    forRange i 100
        Rects[i] = { (math::random 0 500) (math::random 0 500) 30 30 }
        Colors[i] = { (math::random 0 255) (math::random 0 255) (math::random 0 255) 255 }
        Filled[i] = math::random 0 1
    end
end
```
Copy and Count up! some seconds later fps is at 0! 
I need to hack in Rects[i][0] would by nive like in DreiZehn
but for now i need to find out where the counter get out of sync!
and i guess the free slot lookup is what it makes so slow.
```
MoveRects = function
    forRange i 100
        r = Rects[i]
        r[0] = r[0] + 0.1
        r[1] = r[1] + 0.1
        Rects[i] = r
    end
end
```
        
- [X] add a line counter for ParseError using EOLToken! 

- [X] i tried to change the assign for multidimensional array but writing 
directly to tape is a bitch ;) So this would work i guess: arr[10 10] NOT! 
make a LSquare / RSquare loop!! so we get arr[10][10] like i wanted it. I should 
put this in a extra function ,,,, should work with peek forward 
    - [X] parser loop []
    - [X] VM Save
        - [X] detect dimension
        - [X] save loop with auto create ...
    - [X] VM Load
        - [X] detect dimension
        - [X] load loop  - heavy shit and save is harder lol - will by slow i guess
    
- [X] forRange count as argument        


- [X] math/str also as header with IMPL
    
    
- [X] changed VM loop to threaded .. nearly as fast as DreiZehn ByteCode in BaseFlux :)
    
- [X] changed from union Slot to flat U32
- [X] hybrid header only or lib

- [ ] add help for functions and globals 
    
    
## maybe:
- add a callframe too to enable real functions 
- Save a Tape:
    - SymbolTable
    - StringTable
    - Variables
    - tape
