#ifndef SCOPE_H
#define SCOPE_H

#include <parser.h>

typedef struct Scope Scope;

typedef struct Scope {
    Scope* parent;
    char* variables_k[256];
    Value variables_v[256];
    uint32_t varcount;

    char* types_k[256];
    Struct structs[256];
    uint32_t typecount;
    uint32_t structcount;
} Scope;

Scope create_scope(Scope* parent);
void free_scope(Scope* scope);
void scope_declare_var(Scope* scope, char* name);
void scope_define_var(Scope* scope, char* name, Value value);
Value scope_get_var(Scope* scope, char* name);

#endif
