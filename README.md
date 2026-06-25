# Prior

The Prior programming language. Prior is a metaprogrammable statically-typed system programming language that enables the use of runtime modifications (A.K.A Runtime Metaprogramming) while also designed to be *yet another improvement to the C programming language*.

## How are you going to make this work?

The compiler will have a toggle option "metaprogrammable runtime". If true, the compiler will generate the program's object code with additional information called Meta Information (like struct, enum and function headers), how's that going to work? I don't know, I'm learning as I'm doing. The user will be also able to let the compiler generate the Meta Information in a separate file.

Now as I am writing this, I start to feel like this is very useless and could be replaced by DLLs and code header files. But that's the first part of metaprogrammable runtime.

The second part of the metaprogrammable runtime is function loading and patching. This is a work for the user too: choose which functions to patch and which functions to load, what functions even are stored, etc. Now you might be thinking: why not just use C# for that? Yes, C# uses CLR (Common Language Runtime) that allows code patching, it's good, but you know, C# has its own quirks like GC and auto-allocation, and I don't even know if it's optimized.

The goal for this programming language is to have fast metaprogrammable runtime, while also being as close to native execution as possible.

## Current features:
- Fully working Lexer
- Fully working Parser
- Half-implemented Semantics Check
- Barely-implemented Bytecode compilation
- Not implemented at all Assembly Generator.
- Not implemented at all Lenny.

## TODO:
- [x] Tokens
  - [x] Add Positions to Tokens
- [ ] AST
  - [ ] Combine Update Expression with Unary Expression if needed
  - [x] Implement `else if` and `else`
  - [x] Make `if`, `while` statements use Expression instead of just Block
  - [x] Array type expression `[]type`
  - [x] Array expression `[1, 2, 3, ...]`
  - [x] Implement error system
  - [x] Implement `break` and `continue`
  - [x] Switch statement `switch (<expr>) <switch-block>`
  - [x] Implement enum
  - [ ] Switch from memory arena to allocation tree to allow AST optimizations
  - [ ] Implement casts
- [ ] Semantics Check
  - [x] Implement error system
  - [ ] Implement auto-cast
- [ ] Optimizer
  - [ ] Comptime binary expression solver
  - [ ] Comptime unary expression solver
- [x] Bytecode
  - [x] Use a variable instruction size instead of a fixed one
- [x] VM
  - [x] Implement error system

- [ ] Bytecode
  - [ ] Make the Bytecode Structure compatible with assembly to allow Assembly Generation
    - [ ] Implement registers
- [ ] Assembly Generator

## Syntax

```rs
import "std.io";

fn main (): void {
    const string: []c8 = "World"
    std.io.println("Hello {}!" $ string);
}
```
