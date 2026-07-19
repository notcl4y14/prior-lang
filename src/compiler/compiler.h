#ifndef COMPILER_H
#define COMPILER_H

#include <stdbool.h>

typedef struct CompileOptions {
    const char* file;
    bool show_stages;
    bool show_tokens;
    bool show_ast;
    bool show_symtable;
    bool show_ir;
    bool show_asm;
} CompileOptions;

void compile(const CompileOptions* options);

#endif
