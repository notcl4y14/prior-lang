#include "error.h"
#include "scope.h"
#include "token.h"
#include "type.h"
#include "value.h"
#include "parser.h"
#include <assert.h>
#include <semantics.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***
 * SemRes - Semantical Result.
 * Contains the result of the semantical process
 * and determines if there was an error during analysis.
 */
typedef struct SemRes {
    Type type;
    bool error;
} SemRes;

SemRes process_node(Semantics* s, Scope* scope, Node* node);

SemRes process_integer_lit(Semantics* s, Scope* scope, Node* node) {
    return (SemRes) { create_value_typedef(VT_INT32), false };
}

SemRes process_float_lit(Semantics* s, Scope* scope, Node* node) {
    return (SemRes) { create_value_typedef(VT_FLOAT32), false };
}

SemRes process_ident_lit(Semantics* s, Scope* scope, Node* node) {
    NIdentLit ident_lit = node->data.ident_lit;
    Value value = scope_get_var(scope, ident_lit.value);

    if (value.type == VT_NONE) {
        char errmsg[512] = { 0 };
        sprintf(errmsg, "Accessing undefined variable \"%s\"", ident_lit.value);
        semantics_add_error(s, errmsg, node->left_pos);
        return (SemRes) { { 0 }, true };
    }

    Type type = scope_get_var_type(scope, ident_lit.value);

    return (SemRes) { type, false };
}

SemRes process_return_stat(Semantics* s, Scope* scope, Node* node) {
    NRetStat ret_stat = node->data.ret_stat;

    if (ret_stat.expr == NULL) {
        return (SemRes) { { 0 }, false };
    }

    return process_node(s, scope, ret_stat.expr);
}

SemRes process_break_stat(Semantics* s, Scope* scope, Node* node) {
    return (SemRes) { { 0 }, false };
}

SemRes process_continue_stat(Semantics* s, Scope* scope, Node* node) {
    return (SemRes) { { 0 }, false };
}

SemRes process_defer_stat(Semantics* s, Scope* scope, Node* node) {
    NDeferStat defer_stat = node->data.defer_stat;

    return process_node(s, scope, defer_stat.expr);
}

SemRes process_var_stat(Semantics* s, Scope* scope, Node* node) {
    NVarStat var_stat = node->data.var_stat;

    char* ident_name = var_stat.ident->data.ident_lit.value;
    char* type_name = var_stat.type->data.ident_lit.value;

    // TODO: Implement a hashmap that stores types and their names
    Type variable_type = scope_get_type(scope, type_name);

    if (variable_type.type == TYPE_TYPE_NONE) {
        // TODO: bad code, sprintf, fix.
        char emsg[512] = {0};
        sprintf(emsg, "Type '%s' is not defined!", type_name);
        semantics_add_error(s, emsg, var_stat.type->left_pos);
        return (SemRes) { { 0 }, true };
    }

    if (var_stat.value != NULL) {
        SemRes result = process_node(s, scope, var_stat.value);
        if (result.error) return result;

        // TODO: Implement type-value comparison
    }

    /* Storing the variable in the scope */
    scope_declare_var(scope, ident_name, variable_type);
    scope_define_var(scope, ident_name, (Value) { .type = get_typedef_value_type(variable_type), {} });

    return (SemRes) { variable_type, false };
}

SemRes process_fn_stat(Semantics* s, Scope* scope, Node* node) {
    SemRes result;

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
            sprintf(emsg, "Type '%s' is not defined!", param_type_str);
            semantics_add_error(s, emsg, param->type->left_pos);
            return (SemRes) { { 0 }, true };
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

    result = process_node(s, &sub_scope, func_stat.body);
    if (result.error) return result;

    free_scope(&sub_scope);

    return (SemRes) { { 0 }, true };
}


SemRes process_if_stat(Semantics* s, Scope* scope, Node* node) {
    SemRes result;

    NIfStat if_stat = node->data.if_stat;

    result = process_node(s, scope, if_stat.condition);
    if (result.error) return result;

    result = process_node(s, scope, if_stat.body);
    if (result.error) return result;

    if (if_stat.alternate != NULL) {
        result = process_node(s, scope, if_stat.alternate);
        if (result.error) return result;
    }

    return (SemRes) { { 0 }, true };
}

SemRes process_while_stat(Semantics* s, Scope* scope, Node* node) {
    SemRes result;

    NWhileStat while_stat = node->data.while_stat;

    result = process_node(s, scope, while_stat.condition);
    if (result.error) return result;

    result = process_node(s, scope, while_stat.body);
    if (result.error) return result;

    return (SemRes) { { 0 }, false };
}

SemRes process_block(Semantics* s, Scope* scope, Node* node) {
    SemRes result;

    NBlock block = node->data.block;

    Scope sub_scope = create_scope(scope);

    Type last_type = (Type) { 0 };

    for (int32_t i = 0; i < block.nodes.count; ++i) {
        result = process_node(s, &sub_scope, &block.nodes.nodes[i]);
        if (result.error) return result;

        // TODO: Take type only from returns
        last_type = result.type;
    }

    free_scope(&sub_scope);

    return (SemRes) { last_type, false };
}

SemRes process_bin_expr(Semantics* s, Scope* scope, Node* node) {
    SemRes result;

    NBinExpr bin_expr = node->data.bin_expr;

    Node* left = bin_expr.left;
    result = process_node(s, scope, left);
    if (result.error) return result;

    Type left_type = result.type;

    Node* right = bin_expr.right;
    result = process_node(s, scope, right);
    if (result.error) return result;

    // ValueType right_type = process_node(s, right);

    // if (left_type != right_type) {
    //     sprintf(s->errmsg, "Cannot do an arithmetical expression with %s and %s", ValueTypeNames[left_type], ValueTypeNames[right_type]);
    //     s->error = true;

    //     return VT_NONE;
    // }

    // data->return_type = left_type;

    return (SemRes) { left_type, false };
}

SemRes process_update_expr(Semantics* s, Scope* scope, Node* node) {
    NUpdateExpr update_expr = node->data.update_expr;

    return process_node(s, scope, update_expr.expr);
}

SemRes process_assign_expr(Semantics* s, Scope* scope, Node* node) {
    NAssignExpr assign_expr = node->data.assign_expr;

    return process_node(s, scope, assign_expr.value);
}

SemRes process_unary_expr(Semantics* s, Scope* scope, Node* node) {
    NUnExpr un_expr = node->data.un_expr;

    return process_node(s, scope, un_expr.expr);
}

SemRes process_call_expr(Semantics* s, Scope* scope, Node* node) {
    // TODO: Check function
    return (SemRes) { { 0 }, false };
}

SemRes process_cast_expr(Semantics* s, Scope* scope, Node* node) {
    return (SemRes) {
        scope_get_type(
            scope,
            node->data.cast_expr.type->data.ident_lit.value
        ),
        false
    };
}

SemRes process_struct_stat(Semantics* s, Scope* scope, Node* node) {
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
            semantics_add_error(s, emsg, field.type->left_pos);
            return (SemRes) { { 0 }, false };
        }

        type_struct_data.fields_names[i] = field_name;
        type_struct_data.fields_types[i] = field_type;
        type_struct_data.count++;
    }

    const char* struct_ident = struct_stat.ident->data.ident_lit.value;
    type_table_assign_type(&scope->type_table, struct_ident, create_struct_typedef(type_struct_data));

    return (SemRes) { { 0 }, false };
}

SemRes process_node(Semantics* s, Scope* scope, Node* node) {
    switch (node->type) {
        case NT_INTEGER_LIT: return process_integer_lit(s, scope, node);
        case NT_FLOAT_LIT:   return process_float_lit(s, scope, node);
        case NT_IDENT_LIT:   return process_ident_lit(s, scope, node);

        case NT_RETURN_STAT:   return process_return_stat(s, scope, node);
        case NT_BREAK_STAT:    return process_break_stat(s, scope, node);
        case NT_CONTINUE_STAT: return process_continue_stat(s, scope, node);
        case NT_DEFER_STAT:    return process_defer_stat(s, scope, node);
        case NT_VAR_STAT:      return process_var_stat(s, scope, node);
        case NT_FUNC_STAT:     return process_fn_stat(s, scope, node);
        case NT_IF_STAT:       return process_if_stat(s, scope, node);
        case NT_WHILE_STAT:    return process_while_stat(s, scope, node);

        case NT_BLOCK: return process_block(s, scope, node);
        case NT_FIELD: assert(false); // TODO: implement

        case NT_BIN_EXPR:    return process_bin_expr(s, scope, node);
        case NT_UN_EXPR:     return process_unary_expr(s, scope, node);
        case NT_UPDATE_EXPR: return process_update_expr(s, scope, node);
        case NT_ASSIGN_EXPR: return process_assign_expr(s, scope, node);
        case NT_CALL_EXPR:   return process_call_expr(s, scope, node);
        case NT_CAST_EXPR:   return process_cast_expr(s, scope, node);
        case NT_STRUCT_STAT: return process_struct_stat(s, scope, node);

        default:
            printf("Unhandled semantics node type: %s\n", NodeTypeNames[node->type]);
            assert(false);
            return (SemRes) { 0 };
    }
}



Semantics create_semantics(Scope* scope) {
    return (Semantics) {
        .error_list = create_error_list(),
        .scope = scope,
    };
}

void semantics_add_error(Semantics* s, const char* errmsg, TokenPosition position) {
    add_to_error_list(&s->error_list, create_error(errmsg, position));
}

void process_semantics(Semantics* s, Node* ast) {
    NProgram program = ast->data.program;

    for (int32_t i = 0; i < program.nodes.count; ++i) {
        Node* node = &program.nodes.nodes[i];
        process_node(s, s->scope, node);
    }
}
