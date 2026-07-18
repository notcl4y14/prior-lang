# Compiler

The Prior compiler currently takes one file from user's input and executes the next algorithm:

1. Read the file;
2. Lexer: Generate tokens from the file's contents;
  - Aborts the compilation on error;
3. AST Parser: Generate AST from the tokens;
  - Aborts the compilation on error;
4. Semantics: Generate definitions, types, check types from the AST;
  - Aborts the compilation on error;
5. AAR Parser: Generate AAR from the AST and the type table;
  - Does not present any possibility of giving errors yet;
6. Assembly Generation: Generate Assembly code from the AAR, with specific assembly language, target and architecture;
  - Does not present any possibility of giving errors yet;
7. Execution of external tools;
  1. Assembler (depends on the chosen assembly language output);
    - Presents possibility of giving errors, which would abort the compilation;
  2. Linker;
    - Definitely presents possibility of giving errors, which would definitely abort the compilation.

When the compiler has successfully finished its job, it should output the binary file the user specified.
The entire algorithm of the compiler's execution is stored in `src/main.c` in the `void compile()` function.
