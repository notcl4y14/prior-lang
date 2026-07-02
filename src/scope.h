#ifndef SCOPE_H
#define SCOPE_H

#include "type.h"
#include <parser.h>
#include <stddef.h>

typedef struct Scope Scope;

typedef struct Scope {
    Scope* parent;
    char* variables_k[256];
    Value variables_v[256];
    Type  variables_t[256];
    uint32_t varcount;

    TypeTable type_table;
    Node defers[256];
    size_t defer_count;
    bool is_deferred;
} Scope;

Scope create_scope(Scope* parent);
void free_scope(Scope* scope);
void scope_declare_var(Scope* scope, char* name, Type type);
void scope_define_var(Scope* scope, char* name, Value value);
Value scope_get_var(Scope* scope, char* name);
Type scope_get_var_type(Scope* scope, char* name);
void print_scope_structs(Scope* scope);
void scope_add_defer(Scope* scope, Node node);

/* Returns type from TypeTable, but searches further */
Type scope_get_type(Scope* tt, const char* ident);

#endif
