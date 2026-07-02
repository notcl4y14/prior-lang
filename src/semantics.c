#include "scope.h"
#include "type.h"
#include "value.h"
#include "parser.h"
#include <assert.h>
#include <semantics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Type process_node(Semantics* s, Scope* scope, Node* node);

Type process_integer_lit(Semantics* s, Scope* scope, Node* node) {
    return create_value_typedef(VT_INT32);
}

Type process_float_lit(Semantics* s, Scope* scope, Node* node) {
    return create_value_typedef(VT_NONE);
}

Type process_ident_lit(Semantics* s, Scope* scope, Node* node) {
    NIdentLit ident_lit = node->data.ident_lit;
    Value value = scope_get_var(scope, ident_lit.value);

    if (value.type == VT_NONE) {
        char errmsg[512] = { 0 };
        sprintf(errmsg, "Accessing undefined variable \"%s\"", ident_lit.value);
        set_semantics_error(s, errmsg);
        return (Type) { 0 };
    }

    Type type = scope_get_var_type(scope, ident_lit.value);

    return type;
}

Type process_return_stat(Semantics* s, Scope* scope, Node* node) {
    NRetStat ret_stat = node->data.ret_stat;

    if (ret_stat.expr == NULL) {
        // ret_stat.return_type = VT_NONE;
        return (Type) { 0 };
    }

    // data->return_type = process_node(s, data->value);
    // return data->return_type;
    return process_node(s, scope, ret_stat.expr);
}

Type process_break_stat(Semantics* s, Scope* scope, Node* node) {
    return (Type) { 0 };
}

Type process_continue_stat(Semantics* s, Scope* scope, Node* node) {
    return (Type) { 0 };
}

// static bool struct_exists(Semantics* s, char* struct_name) {
//     for (uint32_t i = 0; i < s->scope->typecount; i++) {
//         if (!strcmp(s->scope->types_k[i], struct_name)) {
//             return true;
//         }
//     }
//     return false;
// }

Type process_defer_stat(Semantics* s, Scope* scope, Node* node) {
    NDeferStat defer_stat = node->data.defer_stat;

    return process_node(s, scope, defer_stat.expr);
}

Type process_var_stat(Semantics* s, Scope* scope, Node* node) {
    NVarStat var_stat = node->data.var_stat;

    char* ident_name = var_stat.ident->data.ident_lit.value;
    char* type_name = var_stat.type->data.ident_lit.value;

    // TODO: Implement a hashmap that stores types and their names
    Type variable_type = scope_get_type(scope, type_name);

    if (variable_type.type == TYPE_TYPE_NONE) {
        // TODO: bad code, sprintf, fix.
        char emsg[512] = {0};
        sprintf(emsg, "Type '%s' is not defined! At line %ld", type_name, node->left_pos.line);
        set_semantics_error(s, emsg);
        assert(false);
        return (Type) { 0 };
    }

    // ValueType value_type = process_node(s, data->value);

    // printf("%s\n", ValueTypeNames[variable_type]);
    // data->return_type = variable_type;

    /* Storing the variable in the scope */
    scope_declare_var(scope, ident_name, variable_type);
    scope_define_var(scope, ident_name, (Value) { .type = get_typedef_value_type(variable_type), {} });

    return variable_type;
}

Type process_fn_stat(Semantics* s, Scope* scope, Node* node) {
    NFuncStat func_stat = node->data.func_stat;

    Scope sub_scope = create_scope(scope);

    for (int32_t i = 0; i < func_stat.params.count; ++i) {
        NParameter* param = &func_stat.params.nodes[i].data.parameter;
        char* param_ident_str = param->ident->data.ident_lit.value;
        char* param_type_str = param->type->data.ident_lit.value;

        Type param_type = scope_get_type(scope, param_type_str);

        if (param_type.type == TYPE_TYPE_NONE) {
            // TODO: bad code, sprintf, fix.
            char emsg[512] = {0};
            sprintf(emsg, "Type '%s' is not defined! At line %ld", param_type_str, node->left_pos.line);
            set_semantics_error(s, emsg);
            assert(false);
            return (Type) { 0 };
        }

        scope_declare_var(&sub_scope, param_ident_str, param_type);
        scope_define_var(
            &sub_scope,
            param_ident_str,
            (Value) {
                .type = get_typedef_value_type(param_type),
                {}
            }
        );
    }

    process_node(s, &sub_scope, func_stat.body);

    free_scope(&sub_scope);

    return (Type) { 0 };
}


Type process_if_stat(Semantics* s, Scope* scope, Node* node) {
    NIfStat if_stat = node->data.if_stat;

    process_node(s, scope, if_stat.condition);
    process_node(s, scope, if_stat.body);

    if (if_stat.alternate != NULL) {
        process_node(s, scope, if_stat.alternate);
    }

    return (Type) { 0 };
}

Type process_while_stat(Semantics* s, Scope* scope, Node* node) {
    NWhileStat while_stat = node->data.while_stat;

    process_node(s, scope, while_stat.condition);
    process_node(s, scope, while_stat.body);

    return (Type) { 0 };
}

Type process_block(Semantics* s, Scope* scope, Node* node) {
    NBlock block = node->data.block;

    Scope sub_scope = create_scope(scope);

    Type last_type = (Type) { 0 };

    for (int32_t i = 0; i < block.nodes.count; ++i) {
        last_type = process_node(s, &sub_scope, &block.nodes.nodes[i]);
        if (s->error) return (Type) { 0 };
    }

    free_scope(&sub_scope);

    return last_type;
}

Type process_bin_expr(Semantics* s, Scope* scope, Node* node) {
    NBinExpr bin_expr = node->data.bin_expr;

    // int + int = int
    // float + float = float
    // int + float = int
    // float + int = float

    // TODO: Implement auto-casting

    Node* left = bin_expr.left;
    Type left_type = process_node(s, scope, left);

    Node* right = bin_expr.right;
    process_node(s, scope, right);
    // ValueType right_type = process_node(s, right);

    // if (left_type != right_type) {
    //     sprintf(s->errmsg, "Cannot do an arithmetical expression with %s and %s", ValueTypeNames[left_type], ValueTypeNames[right_type]);
    //     s->error = true;

    //     return VT_NONE;
    // }

    // data->return_type = left_type;

    return left_type;
}

Type process_update_expr(Semantics* s, Scope* scope, Node* node) {
    NUpdateExpr update_expr = node->data.update_expr;
    // data->return_type = process_node(s, data->expr);

    // return data->return_type;
    return process_node(s, scope, update_expr.expr);
}

Type process_assign_expr(Semantics* s, Scope* scope, Node* node) {
    NAssignExpr assign_expr = node->data.assign_expr;
    // data->return_type = process_node(s, data->value);

    // return data->return_type;
    return process_node(s, scope, assign_expr.value);
}

Type process_unary_expr(Semantics* s, Scope* scope, Node* node) {
    NUnExpr un_expr = node->data.un_expr;
    // data->return_type = process_node(s, data->expr);

    // return data->return_type;
    return process_node(s, scope, un_expr.expr);
}

Type process_call_expr(Semantics* s, Scope* scope, Node* node) {
    return (Type) { 0 };
}

Type process_cast_expr(Semantics* s, Scope* scope, Node* node) {
    return scope_get_type(
        scope,
        node->data.cast_expr.type->data.ident_lit.value
    );
}

Type process_struct_stat(Semantics* s, Scope* scope, Node* node) {
    NStructStat struct_stat = node->data.struct_stat;

    TypeStructData type_struct_data = create_type_struct_data();

    // Process each field and assign them types
    for (size_t i = 0; i < struct_stat.fields.count; i++) {
        NField field = struct_stat.fields.nodes[i].data.field;

        char* field_name = field.ident->data.ident_lit.value;
        char* field_type = field.type->data.ident_lit.value;

        /* Check if the type exists in the Type Table */
        if (scope_get_type(scope, field_type).type == TYPE_TYPE_NONE) {
            // TODO: bad code, sprintf, fix.
            char emsg[512] = {0};
            sprintf(emsg, "Undefined \"%s\" type", field_type);
            set_semantics_error(s, emsg);
            return (Type) { 0 };
        }

        type_struct_data.fields_names[i] = field_name;
        type_struct_data.fields_types[i] = field_type;
        type_struct_data.count++;
    }

    const char* struct_ident = struct_stat.ident->data.ident_lit.value;
    type_table_assign_type(&scope->type_table, struct_ident, create_struct_typedef(type_struct_data));

    return (Type) { 0 };
}

Type process_node(Semantics* s, Scope* scope, Node* node) {
    switch (node->type) {
        case NT_INTEGER_LIT:
            return process_integer_lit(s, scope, node);

        case NT_FLOAT_LIT:
            return process_float_lit(s, scope, node);

        case NT_IDENT_LIT:
            return process_ident_lit(s, scope, node);


        case NT_RETURN_STAT:
            return process_return_stat(s, scope, node);

        case NT_BREAK_STAT:
            return process_break_stat(s, scope, node);

        case NT_CONTINUE_STAT:
            return process_continue_stat(s, scope, node);

        case NT_DEFER_STAT:
            return process_defer_stat(s, scope, node);

        case NT_VAR_STAT:
            return process_var_stat(s, scope, node);

        case NT_FUNC_STAT:
            return process_fn_stat(s, scope, node);

        case NT_IF_STAT:
            return process_if_stat(s, scope, node);

        case NT_WHILE_STAT:
            return process_while_stat(s, scope, node);


        case NT_BLOCK:
            return process_block(s, scope, node);


        case NT_BIN_EXPR:
            return process_bin_expr(s, scope, node);

        case NT_UN_EXPR:
            return process_unary_expr(s, scope, node);

        case NT_UPDATE_EXPR:
            return process_update_expr(s, scope, node);

        case NT_ASSIGN_EXPR:
            return process_assign_expr(s, scope, node);

        case NT_CALL_EXPR:
            return process_call_expr(s, scope, node);

        case NT_CAST_EXPR:
            return process_cast_expr(s, scope, node);

        case NT_STRUCT_STAT:
            return process_struct_stat(s, scope, node);

        case NT_FIELD:
            assert(false); // TODO: implement

        default:
            printf("Unhandled semantics node type: %s\n", NodeTypeNames[node->type]);
            assert(false);
            return (Type) { 0 };
    }
}



void set_semantics_error(Semantics* s, const char* errmsg) {
    strncpy(s->errmsg, errmsg, SEMANTICS_ERROR_SIZE);
    s->error = true;
}

const char* get_semantics_error(Semantics* s) {
    if (!s->error) {
        return NULL;
    }

    return s->errmsg;
}

Semantics create_semantics(Scope* scope) {
    return (Semantics) {
        .errmsg = { 0 },
        .error = false,
        .scope = scope
    };
}

void process_semantics(Semantics* s, Node* ast) {
    NProgram program = ast->data.program;

    for (int32_t i = 0; i < program.nodes.count; ++i) {
        Node* node = &program.nodes.nodes[i];
        process_node(s, s->scope, node);
        if (s->error) break;
    }
}
