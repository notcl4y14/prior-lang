#include "scope.h"
#include "mem.h"
#include "type.h"
#include "value.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

Scope create_scope(Scope* parent) {
    Scope scope = (Scope) { 0 };
    scope.parent = parent;
    scope.type_table = create_type_table();
    scope.defer_count = 0;
    scope.is_deferred = false;
    return scope;
}

void free_scope(Scope* scope) {
    for (int32_t i = 0; i < scope->varcount; ++i) {
        if (scope->variables_k[i] != NULL) {
            free(scope->variables_k[i]);
        }
    }

    free_type_table(&scope->type_table);
}

void scope_declare_var(Scope* scope, char* name, Type type) {
    for (int32_t i = 0; i < 256; ++i) {
        if (scope->variables_k[i] == NULL) {
            scope->variables_k[i] = str_alloc_copy(name);
            scope->variables_t[i] = type;
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

    return (Value) { 0 };
}

Type scope_get_var_type(Scope* scope, char* name) {
    for (int32_t i = 0; i < scope->varcount; ++i) {
        if (strcmp(scope->variables_k[i], name) == 0) {
            return scope->variables_t[i];
        }
    }

    if (scope->parent != NULL) {
        return scope_get_var_type(scope->parent, name);
    }

    return (Type) { 0 };
}

void print_scope_structs(Scope* scope) {
    for (int32_t i = 0; i < scope->type_table.count; ++i) {
        const Type* type = &scope->type_table.types_values[i];

        if (type->type != TYPE_TYPE_STRUCT) {
            continue;
        }

        const char* type_ident = scope->type_table.types_idents[i];
        const TypeStructData* struct_ = &type->data.data_struct;

        printf("struct %s {\n", type_ident);

        for (int32_t j = 0; j < struct_->count; ++j) {
            printf("\t%s: %s,\n", struct_->fields_names[j], struct_->fields_types[j]);
        }

        printf("}\n");
    }
}

void scope_add_defer(Scope* scope, Node node) {
    scope->defers[scope->defer_count++] = node;
}

Type scope_get_type(Scope* tt, const char* ident) {
    Type type = type_table_get_type(&tt->type_table, ident);

    if (type.type == TYPE_TYPE_NONE) {
        if (tt->parent != NULL) {
            type = scope_get_type(tt->parent, ident);
        }
    }

    return type;
}
