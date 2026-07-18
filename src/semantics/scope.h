#ifndef SCOPE_H
#define SCOPE_H

#include "parser/parser.h"
#include "type.h"

#include <stddef.h>

typedef struct Scope Scope;

typedef struct Scope {
    Scope* parent;

    char*    variables_k[256];
    Value    variables_v[256];
    Type     variables_t[256];
    uint32_t varcount;

    DefTable def_table;

    Node   defers[256];
    size_t defer_count;
    bool   is_deferred;
} Scope;

Scope create_scope(Scope* parent);
void free_scope(Scope* scope);

void scope_declare_var(Scope* scope, const char* name, Type type);
void scope_define_var(Scope* scope, const char* name, Value value);
Value scope_get_var(Scope* scope, const char* name);
Type scope_get_var_type(Scope* scope, const char* name);

void scope_add_defer(Scope* scope, Node node);

void print_scope_functions(Scope* scope);
void print_scope_structs(Scope* scope);
void print_scope_enums(Scope* scope);

/* Returns type from the DefTable, but searches further into parent scopes */
Definition scope_get_def(Scope* scope, const char* ident);

#endif
