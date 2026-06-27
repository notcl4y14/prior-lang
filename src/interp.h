#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <parser.h>
#include <value.h>

typedef struct Scope {
    char* variables_k[256];
    Value variables_v[256];
} Scope;

Scope create_scope();
void free_scope(Scope* scope);
void scope_declare_var(Scope* scope, char* name);
void scope_define_var(Scope* scope, char* name, Value value);
Value scope_get_var(Scope* scope, char* name);

typedef struct Interpreter {
    Node  ast;
    Scope scope;
} Interpreter;

Interpreter create_interpreter(Node ast);
void free_interpreter(Interpreter* interp);
void run_interpreter(Interpreter* interp);

#endif
