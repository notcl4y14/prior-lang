#include "scope.h"
#include "mem.h"
#include <assert.h>
#include <string.h>

Scope create_scope(Scope* parent) {
    Scope scope = (Scope) { 0 };
    scope.parent = parent;
    return scope;
}

void free_scope(Scope* scope) {
    for (int32_t i = 0; i < scope->varcount; ++i) {
        if (scope->variables_k[i] != NULL) {
            free(scope->variables_k[i]);
        }
    }
}

void scope_declare_var(Scope* scope, char* name) {
    for (int32_t i = 0; i < 256; ++i) {
        if (scope->variables_k[i] == NULL) {
            scope->variables_k[i] = str_alloc_copy(name);
            scope->varcount++;
            return;
        }
    }

    assert(false);
}

void scope_define_var(Scope* scope, char* name, Value value) {
    for (int32_t i = 0; i < scope->varcount; ++i) {
        if (strcmp(scope->variables_k[i], name) == 0) {
            scope->variables_v[i] = value;
            return;
        }
    }

    if (scope->parent != NULL) {
        return scope_define_var(scope->parent, name, value);
    }

    assert(false);
}

Value scope_get_var(Scope* scope, char* name) {
    for (int32_t i = 0; i < scope->varcount; ++i) {
        if (strcmp(scope->variables_k[i], name) == 0) {
            return scope->variables_v[i];
        }
    }

    if (scope->parent != NULL) {
        return scope_get_var(scope->parent, name);
    }

    assert(false);
    return (Value) { 0 };
}
