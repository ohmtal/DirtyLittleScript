# Parser

Since I worked and documented the DreiZehn parser i want to do the same here, 
since it's a bit different and since i made this without a AST it need some docu.
As I did this after DreiZehn it's much cleaner - have less Tokens - but also
push the Bytecode. But with the missing AST it's less flexible. 
**TokenType::Identifier** is the most unflexible part since it looks forward
to get the Variables and Assign build correctly together. 

## Overview

```
::parse => parseStatements => parseMath => parsePrimary |
                                 ^ ----------------------
```
## ::parse - the entry point 

Does only loop the the Tokens and call *parseStatements*.
If fail it rollback (rewind) and return false. Since i handle the "Body" 
Statements using parseStatements inside i do not need this Block handling 
I do in DreiZehn - this makes it more robust and better readable.

## ::parseStatements

- EOL and Semicolon
- If
- While
- ForRange
- Continue
- Break 
- Return

## :parseMath

- Handle BackSlash
- does precedence check 
- drop InlineMathAssign (handled in Identifier)
- Inline Math special handling (need only one parameter)
- *switch* for al math types

## :parsePrimary
- Number (Literal)
- StringLiteral
- LParen
- Identifier 
- Function
- Curly 
- EOL and Semicolon (again)
- Default: cause error and exit !

## ::getArg

My universal Call to get a Argument. It calls "parseMath" check if it got somethin
and push the request counter - which is needed to return a value or not. 
