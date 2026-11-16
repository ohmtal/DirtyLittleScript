# The one billion counter benchmark

Some time ago I saw there are people who run a empty loop like the following
to Benchmark a programming language:

```
forRange i 1000000000
end
print "i=" i
```

So here is the Dirty Little Script Result:

```
Performance counter stats for './dlsDemo scripts/counter.dls':

         2,370,328      cycles:u                                                              
         3,032,190      instructions:u                                                        
           552,392      branches:u                                                            
            19,882      branch-misses:u                                                       
            16,954      L1-icache-load-misses:u                                               

       0.002103049 seconds time elapsed

       0.000688000 seconds user
       0.001388000 seconds sys


```

I guess the most time of the about 2 milliseconds are consumed before the 
bytecode runs. 

## Why is DLS so fast here ?

Here is my optimized byte code generator function:

```
bool Parser::parseForRange() {
    advance();

    std::string variable = "";
    if (peek().mType != TokenType::Identifier) {
        PrintParseError("forRange need a variable");
        return false;
    } else {
        variable = peek().mValue;
        advance();
    }


    // ----1. emit loop variable and set to -1
    vm->emitCode(op_literal);
    // vm->emitTapeChunk(vm->addLiteral(Value(-1)));
    vm->emitValue(Value(-1));
    U32 symId = SymbolTable::insert(variable);
    size_t varidx = vm->Variables.reserveGlobal(symId);
    vm->emitCode(op_save_global);
    vm->emitTapeChunk((U32)varidx);

    // ----2. emit count
    if (!getArg("ForRange count")) return false;
    vm->emitCode(op_cast_long);

    U32 tapestart = vm->tell();

    // ---- save tape position
    size_t loop_start_slot = vm->tell();

    // ---- increment compare and jump!
    vm->emitCode(op_for_range_jump);
    vm->emitTapeChunk((U32)varidx);

    size_t false_jump_slot = vm->tell();
    vm->emitJumpOffset(0); // false placeholder


    // break continue
    LoopContext context;
    context.continue_target = loop_start_slot;
    vm->mLoopStack.push_back(context);

    U32 tapeBeforeStatements = vm->tell();
    // main loop:
    while (peek().mType != TokenType::EOFToken &&
        peek().mType != TokenType::End)
    {
        if (!parseStatements()){
            PrintParseError("ForRange: failed to parse Statements");
            return false;
        }
    }

    //NOTE no more vm emits until tapeBeforeStatements

    if (peek().mType != TokenType::End) {
        PrintParseError("ForRange missing end!");
        return false;
    }
    advance(); //end

    // cheat alias optimize!! one billion benchmark :P
    // we have NO statement so we do not need to loop here
    // simply save the limit to the iter variable:
    if ( tapeBeforeStatements == vm->tell() ) {
        vm->rewindTape(tapestart);

        vm->emitCode(op_save_global);
        vm->emitTapeChunk((U32)varidx);

        vm->emitCode(op_dec);
        vm->emitTapeChunk((U32)varidx);

        return true;
    }


    vm->emitCode(op_jump);
    vm->emitJumpOffset(loop_start_slot);


    U32 loop_end_slot = vm->tell();
    vm->patchJumpOffset(false_jump_slot,vm->tell());
    vm->emitCode(op_pop_stack); // the limit value

    // handle breaks
    for (size_t break_slot : vm->mLoopStack.back().break_slots) {
        vm->patchJumpOffset(break_slot, loop_end_slot);
    }
    vm->mLoopStack.pop_back();

    return true;
}
```

## To show why this benchmark is so stupid, I optimized my forRange byte code. 

### What does this code ? 

#### My first step is saving the iteration variable to my globals stack as Value -1:
```
    // ----1. emit loop variable and set to -1
    vm->emitCode(op_literal);
    // vm->emitTapeChunk(vm->addLiteral(Value(-1)));
    vm->emitValue(Value(-1));
    U32 symId = SymbolTable::insert(variable);
    size_t varidx = vm->Variables.reserveGlobal(symId);
    vm->emitCode(op_save_global);
    vm->emitTapeChunk((U32)varidx);

```

#### Second I call my argument parser which push the Limit argument. 
```    
    // ----2. emit count
    if (!getArg("ForRange count")) return false;
    vm->emitCode(op_cast_long);
```

When i would check the 1000000000 is a literal which i can directly convert there is 
some more space to optimize. 

#### Optimize begins with tapeBeforeStatements

```
    U32 tapeBeforeStatements = vm->tell();
    // main loop:
    while (peek().mType != TokenType::EOFToken &&
        peek().mType != TokenType::End)
    {
        if (!parseStatements()){
            PrintParseError("ForRange: failed to parse Statements");
            return false;
        }
    }
```

I save the tape position before the inside loop Statements are added. 

#### Here is the "cheat":

After the `end` is detected i check the tape moved after the inside loop Statements.
If not there are no statements. And I do a early out:

```
    if ( tapeBeforeStatements == vm->tell() ) {
        vm->rewindTape(tapestart);

        vm->emitCode(op_save_global);
        vm->emitTapeChunk((U32)varidx);

        vm->emitCode(op_dec);
        vm->emitTapeChunk((U32)varidx);

        return true;
    }
```

The rewindTape remove all the code I added after tapestart position. Then I save
the Limit, which is pushed by getArg, to the iter variable. I also have to 
decrement the iter variable since forRange counts from 0 to limit so it's 
`limit -1`

## So Dirty Little Script should be one of the fastest here *haha*
