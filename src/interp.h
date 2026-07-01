#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include <value.h>
#include "scope.h"

typedef enum EvalBreakType {
    EBT_NONE,
    EBT_RETURN,
    EBT_BREAK,
    EBT_CONTINUE,
} EvalBreakType;

typedef struct EvalResult {
    Value         value;
    EvalBreakType break_type;
} EvalResult;

typedef struct Interpreter {
    Node  ast;
    Scope scope;
} Interpreter;

Interpreter create_interpreter(Node ast);
void free_interpreter(Interpreter* interp);
void run_interpreter(Interpreter* interp);

#endif
