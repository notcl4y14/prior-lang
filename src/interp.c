#include "value_types.h"
#include <interp.h>
#include <mem.h>
#include <parser.h>
#include <token.h>
#include <value.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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



EvalResult evaluate_node(Scope* scope, Node node);

EvalResult evaluate_return_stat(Scope* scope, Node node) {
    NodeReturnStatData* data = (NodeReturnStatData*) node.pool_ptr;

    Value value = (Value) { 0 };

    if (data->value.type != NT_NONE) {
        value = evaluate_node(scope, data->value).value;
    }

    return (EvalResult) {
        .value = value,
        .break_type = EBT_RETURN,
    };
}

EvalResult evaluate_break_stat(Scope* scope, Node node) {
    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_BREAK,
    };
}

EvalResult evaluate_continue_stat(Scope* scope, Node node) {
    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_CONTINUE,
    };
}

EvalResult evaluate_var_stat(Scope* scope, Node node) {
    NodeVarStatData* data = (NodeVarStatData*) node.pool_ptr;

    char* name = ((NodeLiteralData*)(data->ident.pool_ptr))->value;

    scope_declare_var(scope, name);

    if (data->value.type != NT_NONE) {
        Value value = evaluate_node(scope, data->value).value;
        value = cast_value(value, data->return_type);

        scope_define_var(scope, name, value);
    }

    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_fn_stat(Scope* scope, Node node) {
    NodeFunctionData* data = (NodeFunctionData*) node.pool_ptr;

    char* name = ((NodeLiteralData*)(data->ident.pool_ptr))->value;
    char* type = ((NodeLiteralData*)(data->type.pool_ptr))->value;

    scope_declare_var(scope, name);

    ValueFunction fn_value = (ValueFunction) {
        .params_names = { 0 },
        .params_types = { 0 },
        .node = data->block,
        .return_type = get_value_type_from_string(type),
    };

    // TODO: Free
    ValueFunction* fn_ptr = malloc(sizeof(ValueFunction));
    *fn_ptr = fn_value;

    // TODO: Improve pointers here
    scope_define_var(scope, name, (Value) { .type = VT_UINT64, .value.u64 = (uint64_t) fn_ptr });

    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_if_stat(Scope* scope, Node node) {
    NodeIfStatData* data = (NodeIfStatData*) node.pool_ptr;

    Value condition = evaluate_node(scope, data->expr).value;

    if (condition.value.u8 == 1) {
        return evaluate_node(scope, data->block);
    } else {
        if (data->ifelse.type != NT_NONE) {
            return evaluate_node(scope, data->ifelse);
        }
    }

    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_while_stat(Scope* scope, Node node) {
    NodeWhileData* data = (NodeWhileData*) node.pool_ptr;

    EvalResult last_result = (EvalResult) { 0 };
    Value condition = evaluate_node(scope, data->expr).value;

    while (condition.value.u8 == 1) {
        EvalResult next_result = evaluate_node(scope, data->block);

        if (next_result.break_type == EBT_RETURN) {
            last_result = next_result;
            break;
        } else if (next_result.break_type == EBT_BREAK) {
            break;
        } else if (next_result.break_type == EBT_CONTINUE) {
            condition = evaluate_node(scope, data->expr).value;
            continue;
        }

        last_result = next_result;
        condition = evaluate_node(scope, data->expr).value;
    }

    return last_result;
}



EvalResult evaluate_block(Scope* scope, Node node) {
    NodeBlockData* data = (NodeBlockData*) node.pool_ptr;

    Scope sub_scope = create_scope(scope);

    EvalResult last_result = (EvalResult) { 0 };

    for (int32_t i = 0; i < data->count; ++i) {
        last_result = evaluate_node(&sub_scope, data->nodes[i]);

        if (last_result.break_type == EBT_RETURN) {
            break;
        }
    }

    free_scope(&sub_scope);

    return last_result;
}



EvalResult evaluate_bin_expr(Scope* scope, Node node) {
    NodeBinExprData* data = (NodeBinExprData*) node.pool_ptr;

    Value left_value = evaluate_node(scope, data->left).value;
    Value right_value = evaluate_node(scope, data->right).value;

    /*  Auto-casting */
    if (left_value.type != right_value.type) {
        right_value = cast_value(right_value, left_value.type);
    }

    Value result_value = (Value) { 0 };
    result_value.type = left_value.type;

    if (data->op == TT_PLUS) {
        switch (result_value.type) {
            case VT_INT8:    result_value.value.i8  = left_value.value.i8  + right_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8  = left_value.value.u8  + right_value.value.u8;  break;

            case VT_INT16:   result_value.value.i16 = left_value.value.i16 + right_value.value.i16; break;
            case VT_UINT16:  result_value.value.u16 = left_value.value.u16 + right_value.value.u16; break;

            case VT_INT32:   result_value.value.i32 = left_value.value.i32 + right_value.value.i32; break;
            case VT_UINT32:  result_value.value.u32 = left_value.value.u32 + right_value.value.u32; break;

            case VT_INT64:   result_value.value.i64 = left_value.value.i64 + right_value.value.i64; break;
            case VT_UINT64:  result_value.value.u64 = left_value.value.u64 + right_value.value.u64; break;

            case VT_FLOAT32: result_value.value.f32 = left_value.value.f32 + right_value.value.f32; break;
            case VT_FLOAT64: result_value.value.f64 = left_value.value.f64 + right_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_MINUS) {
        switch (result_value.type) {
            case VT_INT8:    result_value.value.i8  = left_value.value.i8  - right_value.value.i8;   break;
            case VT_UINT8:   result_value.value.u8  = left_value.value.u8  - right_value.value.u8;   break;

            case VT_INT16:   result_value.value.i16 = left_value.value.i16 - right_value.value.i16; break;
            case VT_UINT16:  result_value.value.u16 = left_value.value.u16 - right_value.value.u16; break;

            case VT_INT32:   result_value.value.i32 = left_value.value.i32 - right_value.value.i32; break;
            case VT_UINT32:  result_value.value.u32 = left_value.value.u32 - right_value.value.u32; break;

            case VT_INT64:   result_value.value.i64 = left_value.value.i64 - right_value.value.i64; break;
            case VT_UINT64:  result_value.value.u64 = left_value.value.u64 - right_value.value.u64; break;

            case VT_FLOAT32: result_value.value.f32 = left_value.value.f32 - right_value.value.f32; break;
            case VT_FLOAT64: result_value.value.f64 = left_value.value.f64 - right_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_ASTERISK) {
        switch (result_value.type) {
            case VT_INT8:    result_value.value.i8  = left_value.value.i8  * right_value.value.i8;   break;
            case VT_UINT8:   result_value.value.u8  = left_value.value.u8  * right_value.value.u8;   break;

            case VT_INT16:   result_value.value.i16 = left_value.value.i16 * right_value.value.i16; break;
            case VT_UINT16:  result_value.value.u16 = left_value.value.u16 * right_value.value.u16; break;

            case VT_INT32:   result_value.value.i32 = left_value.value.i32 * right_value.value.i32; break;
            case VT_UINT32:  result_value.value.u32 = left_value.value.u32 * right_value.value.u32; break;

            case VT_INT64:   result_value.value.i64 = left_value.value.i64 * right_value.value.i64; break;
            case VT_UINT64:  result_value.value.u64 = left_value.value.u64 * right_value.value.u64; break;

            case VT_FLOAT32: result_value.value.f32 = left_value.value.f32 * right_value.value.f32; break;
            case VT_FLOAT64: result_value.value.f64 = left_value.value.f64 * right_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_SLASH) {
        if (right_value.value.u64 == 0) {
            printf("Cannot divide by 0\n");
            exit(1);
        }

        switch (result_value.type) {
            case VT_INT8:    result_value.value.i8  = left_value.value.i8  / right_value.value.i8;   break;
            case VT_UINT8:   result_value.value.u8  = left_value.value.u8  / right_value.value.u8;   break;

            case VT_INT16:   result_value.value.i16 = left_value.value.i16 / right_value.value.i16; break;
            case VT_UINT16:  result_value.value.u16 = left_value.value.u16 / right_value.value.u16; break;

            case VT_INT32:   result_value.value.i32 = left_value.value.i32 / right_value.value.i32; break;
            case VT_UINT32:  result_value.value.u32 = left_value.value.u32 / right_value.value.u32; break;

            case VT_INT64:   result_value.value.i64 = left_value.value.i64 / right_value.value.i64; break;
            case VT_UINT64:  result_value.value.u64 = left_value.value.u64 / right_value.value.u64; break;

            case VT_FLOAT32: result_value.value.f32 = left_value.value.f32 / right_value.value.f32; break;
            case VT_FLOAT64: result_value.value.f64 = left_value.value.f64 / right_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_EQUALS) {
        result_value.type = VT_UINT8;

        switch (left_value.type) {
            case VT_INT8:    result_value.value.u8 = left_value.value.i8  == right_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8 = left_value.value.u8  == right_value.value.u8;  break;

            case VT_INT16:   result_value.value.u8 = left_value.value.i16 == right_value.value.i16; break;
            case VT_UINT16:  result_value.value.u8 = left_value.value.u16 == right_value.value.u16; break;

            case VT_INT32:   result_value.value.u8 = left_value.value.i32 == right_value.value.i32; break;
            case VT_UINT32:  result_value.value.u8 = left_value.value.u32 == right_value.value.u32; break;

            case VT_INT64:   result_value.value.u8 = left_value.value.i64 == right_value.value.i64; break;
            case VT_UINT64:  result_value.value.u8 = left_value.value.u64 == right_value.value.u64; break;

            case VT_FLOAT32: result_value.value.u8 = left_value.value.f32 == right_value.value.f32; break;
            case VT_FLOAT64: result_value.value.u8 = left_value.value.f64 == right_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_NOT_EQUALS) {
        result_value.type = VT_UINT8;

        switch (left_value.type) {
            case VT_INT8:    result_value.value.u8 = left_value.value.i8  != right_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8 = left_value.value.u8  != right_value.value.u8;  break;

            case VT_INT16:   result_value.value.u8 = left_value.value.i16 != right_value.value.i16; break;
            case VT_UINT16:  result_value.value.u8 = left_value.value.u16 != right_value.value.u16; break;

            case VT_INT32:   result_value.value.u8 = left_value.value.i32 != right_value.value.i32; break;
            case VT_UINT32:  result_value.value.u8 = left_value.value.u32 != right_value.value.u32; break;

            case VT_INT64:   result_value.value.u8 = left_value.value.i64 != right_value.value.i64; break;
            case VT_UINT64:  result_value.value.u8 = left_value.value.u64 != right_value.value.u64; break;

            case VT_FLOAT32: result_value.value.u8 = left_value.value.f32 != right_value.value.f32; break;
            case VT_FLOAT64: result_value.value.u8 = left_value.value.f64 != right_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_LESS) {
        result_value.type = VT_UINT8;

        switch (left_value.type) {
            case VT_INT8:    result_value.value.u8 = left_value.value.i8  < right_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8 = left_value.value.u8  < right_value.value.u8;  break;

            case VT_INT16:   result_value.value.u8 = left_value.value.i16 < right_value.value.i16; break;
            case VT_UINT16:  result_value.value.u8 = left_value.value.u16 < right_value.value.u16; break;

            case VT_INT32:   result_value.value.u8 = left_value.value.i32 < right_value.value.i32; break;
            case VT_UINT32:  result_value.value.u8 = left_value.value.u32 < right_value.value.u32; break;

            case VT_INT64:   result_value.value.u8 = left_value.value.i64 < right_value.value.i64; break;
            case VT_UINT64:  result_value.value.u8 = left_value.value.u64 < right_value.value.u64; break;

            case VT_FLOAT32: result_value.value.u8 = left_value.value.f32 < right_value.value.f32; break;
            case VT_FLOAT64: result_value.value.u8 = left_value.value.f64 < right_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_GREATER) {
        result_value.type = VT_UINT8;

        switch (left_value.type) {
            case VT_INT8:    result_value.value.u8 = left_value.value.i8  > right_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8 = left_value.value.u8  > right_value.value.u8;  break;

            case VT_INT16:   result_value.value.u8 = left_value.value.i16 > right_value.value.i16; break;
            case VT_UINT16:  result_value.value.u8 = left_value.value.u16 > right_value.value.u16; break;

            case VT_INT32:   result_value.value.u8 = left_value.value.i32 > right_value.value.i32; break;
            case VT_UINT32:  result_value.value.u8 = left_value.value.u32 > right_value.value.u32; break;

            case VT_INT64:   result_value.value.u8 = left_value.value.i64 > right_value.value.i64; break;
            case VT_UINT64:  result_value.value.u8 = left_value.value.u64 > right_value.value.u64; break;

            case VT_FLOAT32: result_value.value.u8 = left_value.value.f32 > right_value.value.f32; break;
            case VT_FLOAT64: result_value.value.u8 = left_value.value.f64 > right_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_LESS_EQUALS) {
        result_value.type = VT_UINT8;

        switch (left_value.type) {
            case VT_INT8:    result_value.value.u8 = left_value.value.i8  <= right_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8 = left_value.value.u8  <= right_value.value.u8;  break;

            case VT_INT16:   result_value.value.u8 = left_value.value.i16 <= right_value.value.i16; break;
            case VT_UINT16:  result_value.value.u8 = left_value.value.u16 <= right_value.value.u16; break;

            case VT_INT32:   result_value.value.u8 = left_value.value.i32 <= right_value.value.i32; break;
            case VT_UINT32:  result_value.value.u8 = left_value.value.u32 <= right_value.value.u32; break;

            case VT_INT64:   result_value.value.u8 = left_value.value.i64 <= right_value.value.i64; break;
            case VT_UINT64:  result_value.value.u8 = left_value.value.u64 <= right_value.value.u64; break;

            case VT_FLOAT32: result_value.value.u8 = left_value.value.f32 <= right_value.value.f32; break;
            case VT_FLOAT64: result_value.value.u8 = left_value.value.f64 <= right_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_GREATER_EQUALS) {
        result_value.type = VT_UINT8;

        switch (left_value.type) {
            case VT_INT8:    result_value.value.u8 = left_value.value.i8  >= right_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8 = left_value.value.u8  >= right_value.value.u8;  break;

            case VT_INT16:   result_value.value.u8 = left_value.value.i16 >= right_value.value.i16; break;
            case VT_UINT16:  result_value.value.u8 = left_value.value.u16 >= right_value.value.u16; break;

            case VT_INT32:   result_value.value.u8 = left_value.value.i32 >= right_value.value.i32; break;
            case VT_UINT32:  result_value.value.u8 = left_value.value.u32 >= right_value.value.u32; break;

            case VT_INT64:   result_value.value.u8 = left_value.value.i64 >= right_value.value.i64; break;
            case VT_UINT64:  result_value.value.u8 = left_value.value.u64 >= right_value.value.u64; break;

            case VT_FLOAT32: result_value.value.u8 = left_value.value.f32 >= right_value.value.f32; break;
            case VT_FLOAT64: result_value.value.u8 = left_value.value.f64 >= right_value.value.f64; break;

            default: assert(false); break;
        }
    }

    return (EvalResult) {
        .value = result_value,
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_unary_expr(Scope* scope, Node node) {
    NodeUnaryExprData* data = (NodeUnaryExprData*) node.pool_ptr;

    Value expr_value = evaluate_node(scope, data->expr).value;

    Value result_value = (Value) { 0 };
    result_value.type = expr_value.type;

    if (data->op == TT_MINUS) {
        switch (expr_value.type) {
            case VT_INT8:    result_value.value.i8  = -expr_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8  = -expr_value.value.u8;  break;

            case VT_INT16:   result_value.value.i16 = -expr_value.value.i16; break;
            case VT_UINT16:  result_value.value.u16 = -expr_value.value.u16; break;

            case VT_INT32:   result_value.value.i32 = -expr_value.value.i32; break;
            case VT_UINT32:  result_value.value.u32 = -expr_value.value.u32; break;

            case VT_INT64:   result_value.value.i64 = -expr_value.value.i64; break;
            case VT_UINT64:  result_value.value.u64 = -expr_value.value.u64; break;

            case VT_FLOAT32: result_value.value.f32 = -expr_value.value.f32; break;
            case VT_FLOAT64: result_value.value.f64 = -expr_value.value.f64; break;

            default: assert(false); break;
        }
    }

    return (EvalResult) {
        .value = result_value,
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_assign_expr(Scope* scope, Node node) {
    NodeAssignExprData* data = (NodeAssignExprData*) node.pool_ptr;

    Value var_value = evaluate_node(scope, data->ident).value;
    Value assign_value = evaluate_node(scope, data->value).value;

    /*  Auto-casting */
    if (var_value.type != assign_value.type) {
        assign_value = cast_value(assign_value, var_value.type);
    }

    Value result_value = (Value) { 0 };
    result_value.type = var_value.type;

    if (data->op == TT_EQUAL) {
        switch (var_value.type) {
            case VT_INT8:    result_value.value.i8  = assign_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8  = assign_value.value.u8;  break;

            case VT_INT16:   result_value.value.i16 = assign_value.value.i16; break;
            case VT_UINT16:  result_value.value.u16 = assign_value.value.u16; break;

            case VT_INT32:   result_value.value.i32 = assign_value.value.i32; break;
            case VT_UINT32:  result_value.value.u32 = assign_value.value.u32; break;

            case VT_INT64:   result_value.value.i64 = assign_value.value.i64; break;
            case VT_UINT64:  result_value.value.u64 = assign_value.value.u64; break;

            case VT_FLOAT32: result_value.value.f32 = assign_value.value.f32; break;
            case VT_FLOAT64: result_value.value.f64 = assign_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_PLUS_EQUAL) {
        switch (var_value.type) {
            case VT_INT8:    result_value.value.i8  = var_value.value.i8  + assign_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8  = var_value.value.u8  + assign_value.value.u8;  break;

            case VT_INT16:   result_value.value.i16 = var_value.value.i16 + assign_value.value.i16; break;
            case VT_UINT16:  result_value.value.u16 = var_value.value.u16 + assign_value.value.u16; break;

            case VT_INT32:   result_value.value.i32 = var_value.value.i32 + assign_value.value.i32; break;
            case VT_UINT32:  result_value.value.u32 = var_value.value.u32 + assign_value.value.u32; break;

            case VT_INT64:   result_value.value.i64 = var_value.value.i64 + assign_value.value.i64; break;
            case VT_UINT64:  result_value.value.u64 = var_value.value.u64 + assign_value.value.u64; break;

            case VT_FLOAT32: result_value.value.f32 = var_value.value.f32 + assign_value.value.f32; break;
            case VT_FLOAT64: result_value.value.f64 = var_value.value.f64 + assign_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_MINUS_EQUAL) {
        switch (var_value.type) {
            case VT_INT8:    result_value.value.i8  = var_value.value.i8  - assign_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8  = var_value.value.u8  - assign_value.value.u8;  break;

            case VT_INT16:   result_value.value.i16 = var_value.value.i16 - assign_value.value.i16; break;
            case VT_UINT16:  result_value.value.u16 = var_value.value.u16 - assign_value.value.u16; break;

            case VT_INT32:   result_value.value.i32 = var_value.value.i32 - assign_value.value.i32; break;
            case VT_UINT32:  result_value.value.u32 = var_value.value.u32 - assign_value.value.u32; break;

            case VT_INT64:   result_value.value.i64 = var_value.value.i64 - assign_value.value.i64; break;
            case VT_UINT64:  result_value.value.u64 = var_value.value.u64 - assign_value.value.u64; break;

            case VT_FLOAT32: result_value.value.f32 = var_value.value.f32 - assign_value.value.f32; break;
            case VT_FLOAT64: result_value.value.f64 = var_value.value.f64 - assign_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_TIMES_EQUAL) {
        switch (var_value.type) {
            case VT_INT8:    result_value.value.i8  = var_value.value.i8  * assign_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8  = var_value.value.u8  * assign_value.value.u8;  break;

            case VT_INT16:   result_value.value.i16 = var_value.value.i16 * assign_value.value.i16; break;
            case VT_UINT16:  result_value.value.u16 = var_value.value.u16 * assign_value.value.u16; break;

            case VT_INT32:   result_value.value.i32 = var_value.value.i32 * assign_value.value.i32; break;
            case VT_UINT32:  result_value.value.u32 = var_value.value.u32 * assign_value.value.u32; break;

            case VT_INT64:   result_value.value.i64 = var_value.value.i64 * assign_value.value.i64; break;
            case VT_UINT64:  result_value.value.u64 = var_value.value.u64 * assign_value.value.u64; break;

            case VT_FLOAT32: result_value.value.f32 = var_value.value.f32 * assign_value.value.f32; break;
            case VT_FLOAT64: result_value.value.f64 = var_value.value.f64 * assign_value.value.f64; break;

            default: assert(false); break;
        }
    } else if (data->op == TT_DIVIDE_EQUAL) {
        if (assign_value.value.u64 == 0) {
            printf("Cannot divide by 0\n");
            exit(1);
        }

        switch (var_value.type) {
            case VT_INT8:    result_value.value.i8  = var_value.value.i8  / assign_value.value.i8;  break;
            case VT_UINT8:   result_value.value.u8  = var_value.value.u8  / assign_value.value.u8;  break;

            case VT_INT16:   result_value.value.i16 = var_value.value.i16 / assign_value.value.i16; break;
            case VT_UINT16:  result_value.value.u16 = var_value.value.u16 / assign_value.value.u16; break;

            case VT_INT32:   result_value.value.i32 = var_value.value.i32 / assign_value.value.i32; break;
            case VT_UINT32:  result_value.value.u32 = var_value.value.u32 / assign_value.value.u32; break;

            case VT_INT64:   result_value.value.i64 = var_value.value.i64 / assign_value.value.i64; break;
            case VT_UINT64:  result_value.value.u64 = var_value.value.u64 / assign_value.value.u64; break;

            case VT_FLOAT32: result_value.value.f32 = var_value.value.f32 / assign_value.value.f32; break;
            case VT_FLOAT64: result_value.value.f64 = var_value.value.f64 / assign_value.value.f64; break;

            default: assert(false); break;
        }
    }

    scope_define_var(scope, ((NodeLiteralData*)(data->ident.pool_ptr))->value, result_value);

    return (EvalResult) {
        .value = result_value,
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_node(Scope* scope, Node node) {
    switch (node.type) {
        case NT_INTEGER_LIT: {
            NodeLiteralData* data = (NodeLiteralData*) node.pool_ptr;
            Value value = (Value) { 0 };
            value.type = VT_INT32;
            value.value.i32 = atoi(data->value);

            return (EvalResult) { .value = value, .break_type = EBT_NONE };
        } break;

        case NT_FLOAT_LIT: {
            NodeLiteralData* data = (NodeLiteralData*) node.pool_ptr;
            Value value = (Value) { 0 };
            value.type = VT_FLOAT32;
            value.value.f32 = atof(data->value);

            return (EvalResult) { .value = value, .break_type = EBT_NONE };
        } break;

        case NT_IDENT_LIT: {
            NodeLiteralData* data = (NodeLiteralData*) node.pool_ptr;
            Value value = scope_get_var(scope, data->value);
            return (EvalResult) { .value = value, .break_type = EBT_NONE };
        } break;


        case NT_RETURN_STAT:
            return evaluate_return_stat(scope, node);

        case NT_BREAK_STAT:
            return evaluate_break_stat(scope, node);

        case NT_CONTINUE_STAT:
            return evaluate_continue_stat(scope, node);

        case NT_VAR_STAT:
            return evaluate_var_stat(scope, node);

        case NT_FUNCTION_STAT:
            return evaluate_fn_stat(scope, node);

        case NT_IF_STAT:
            return evaluate_if_stat(scope, node);

        case NT_WHILE_STAT:
            return evaluate_while_stat(scope, node);


        case NT_BLOCK_EXPR:
            return evaluate_block(scope, node);


        case NT_BIN_EXPR:
            return evaluate_bin_expr(scope, node);

        case NT_UNARY_EXPR:
            return evaluate_unary_expr(scope, node);

        case NT_ASSIGN_EXPR:
            return evaluate_assign_expr(scope, node);

        default:
            printf("Node type %s not handled in evaluation switch\n", NodeTypeNames[node.type]);
            assert(false);
            return (EvalResult) { 0 };
    }
}



Interpreter create_interpreter(Node ast) {
    Interpreter interp = (Interpreter) {
        .ast = ast,
        .scope = create_scope(NULL),
    };

    return interp;
}

void free_interpreter(Interpreter* interp) {
    free_scope(&interp->scope);
}

void run_interpreter(Interpreter* interp) {
    NodeBlockData* data = (NodeBlockData*) interp->ast.pool_ptr;

    for (int32_t i = 0; i < data->count; ++i) {
        evaluate_node(&interp->scope, data->nodes[i]);
    }

    Value main_fn_ptr = scope_get_var(&interp->scope, "main");
    ValueFunction* main_fn_value = (ValueFunction*) main_fn_ptr.value.u64;
    NodeBlockData* fn_data = (NodeBlockData*) main_fn_value->node.pool_ptr;

    for (int32_t i = 0; i < fn_data->count; ++i) {
        EvalResult result = evaluate_node(&interp->scope, fn_data->nodes[i]);
        Value value = result.value;

        printf("Last evaluation: %s: ", ValueTypeNames[value.type]);
        switch (value.type) {
            case VT_NONE:
                printf("none\n");
                break;

            case VT_INT8:
                printf("%d\n", value.value.i8);
                break;

            case VT_INT16:
                printf("%d\n", value.value.i16);
                break;

            case VT_INT32:
                printf("%d\n", value.value.i32);
                break;

            case VT_INT64:
                printf("%ld\n", value.value.i64);
                break;

            case VT_UINT8:
                printf("%d\n", value.value.u8);
                break;

            case VT_UINT16:
                printf("%d\n", value.value.u16);
                break;

            case VT_UINT32:
                printf("%d\n", value.value.u32);
                break;

            case VT_UINT64:
                printf("%ld\n", value.value.u64);
                break;

            case VT_FLOAT32:
                printf("%f\n", value.value.f32);
                break;

            case VT_FLOAT64:
                printf("%f\n", value.value.f64);
                break;

            default:
                printf("undefined\n");
                break;
        }

        // TODO: Temporary
        if (result.break_type == EBT_RETURN) {
            break;
        }
    }
}
