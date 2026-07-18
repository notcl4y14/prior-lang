#include "semantics/scope.h"
#include "util/mem.h"
#include "type.h"
#include "value.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

Scope create_scope(Scope* parent) {
    Scope scope = (Scope) { 0 };
    scope.parent = parent;
    scope.def_table = create_def_table();
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

    free_def_table(&scope->def_table);
}



void scope_declare_var(Scope* scope, const char* name, Type type) {
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

void scope_define_var(Scope* scope, const char* name, Value value) {
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

Value scope_get_var(Scope* scope, const char* name) {
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

Type scope_get_var_type(Scope* scope, const char* name) {
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



void scope_add_defer(Scope* scope, Node node) {
    scope->defers[scope->defer_count++] = node;
}



void print_scope_functions(Scope* scope) {
    for (int32_t i = 0; i < scope->def_table.count; ++i) {
        const Definition* def = &scope->def_table.defs_data[i];

        if (def->type != DEF_TYPE_FUNCTION) {
            continue;
        }

        const char* def_ident = scope->def_table.defs_idents[i];
        const FuncDefData* func_data = &def->data.data_function;

        printf("fn %s (", def_ident);

        for (int32_t i = 0; i < func_data->count; ++i) {
            const char* param_ident = func_data->params_idents[i];
            const char* param_type  = func_data->params_types[i];

            printf("%s: %s", param_ident, param_type);

            if (i != func_data->count - 1) {
                printf(", ");
            }
        }

        printf(");\n");
    }
}

void print_scope_structs(Scope* scope) {
    for (int32_t i = 0; i < scope->def_table.count; ++i) {
        const Definition* def = &scope->def_table.defs_data[i];

        if (def->type != DEF_TYPE_STRUCT) {
            continue;
        }

        const char* def_ident = scope->def_table.defs_idents[i];
        const StructDefData* struct_data = &def->data.data_struct;

        printf("struct %s {\n", def_ident);

        for (int32_t j = 0; j < struct_data->count; ++j) {
            printf("\t%s: %s,\n", struct_data->fields_idents[j], struct_data->fields_types[j]);
        }

        printf("}\n");
    }
}

void print_scope_enums(Scope* scope) {
    for (int32_t i = 0; i < scope->def_table.count; ++i) {
        const Definition* def = &scope->def_table.defs_data[i];

        if (def->type != DEF_TYPE_ENUM) {
            continue;
        }

        const char* def_ident = scope->def_table.defs_idents[i];
        const EnumDefData* enum_data = &def->data.data_enum;

        printf("enum %s {\n", def_ident);

        for (int32_t j = 0; j < enum_data->count; ++j) {
            printf("\t%s", enum_data->entries_idents[j]);
            // printf(": %s", value_as_string(enum_data->entries_values[j]));
            printf(",\n");
        }

        printf("}\n");
    }
}



Definition scope_get_def(Scope* scope, const char* ident) {
    Definition def = def_table_get_def(&scope->def_table, ident);

    if (def.type == DEF_TYPE_NONE) {
        if (scope->parent != NULL) {
            def = scope_get_def(scope->parent, ident);
        }
    }

    return def;
}
