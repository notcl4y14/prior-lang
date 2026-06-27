#include <interp.h>
#include <mem.h>
#include <parser.h>
#include <token.h>
#include <value.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Scope create_scope() {
    Scope scope = (Scope) { 0 };
    return scope;
}

void free_scope(Scope* scope) {
    for (int32_t i = 0; i < 256; ++i) {
        if (scope->variables_k[i] != NULL) {
            free(scope->variables_k[i]);
        }
    }
}

void scope_declare_var(Scope* scope, char* name) {
    for (int32_t i = 0; i < 256; ++i) {
        if (scope->variables_k[i] == NULL) {
            scope->variables_k[i] = str_alloc_copy(name);
            return;
        }
    }

    assert(false);
}

void scope_define_var(Scope* scope, char* name, Value value) {
    for (int32_t i = 0; i < 256; ++i) {
        if (strcmp(scope->variables_k[i], name) == 0) {
            scope->variables_v[i] = value;
            return;
        }
    }

    assert(false);
}

Value scope_get_var(Scope* scope, char* name) {
    for (int32_t i = 0; i < 256; ++i) {
        if (strcmp(scope->variables_k[i], name) == 0) {
            return scope->variables_v[i];
        }
    }

    assert(false);
    return (Value) { 0 };
}



Value evaluate_node(Scope* scope, Node node);

Value evaluate_return_stat(Scope* scope, Node node) {
    NodeReturnStatData* data = (NodeReturnStatData*) node.pool_ptr;

    return evaluate_node(scope, data->value);
}

Value evaluate_var_stat(Scope* scope, Node node) {
    NodeVarStatData* data = (NodeVarStatData*) node.pool_ptr;

    char* name = ((NodeLiteralData*)(data->ident.pool_ptr))->value;

    scope_declare_var(scope, name);

    if (data->value.type != NT_NONE) {
        Value value = evaluate_node(scope, data->value);
        // printf("%s\n", ValueTypeNames[value.type]);
        value = cast_value(value, data->return_type);
        scope_define_var(scope, name, value);
    }

    return (Value) { 0 };
}

Value evaluate_if_stat(Scope* scope, Node node) {
    NodeIfStatData* data = (NodeIfStatData*) node.pool_ptr;

    Value condition = evaluate_node(scope, data->expr);

    if (condition.value.u8 == 1) {
        return evaluate_node(scope, data->block);
    } else {
        if (data->ifelse.type != NT_NONE) {
            return evaluate_node(scope, data->ifelse);
        }
    }

    return (Value) { 0 };
}

Value evaluate_while_stat(Scope* scope, Node node) {
    NodeWhileData* data = (NodeWhileData*) node.pool_ptr;

    Value last_value = (Value) { 0 };
    Value condition = evaluate_node(scope, data->expr);

    // TODO: Implement return, break and continue for while loop
    while (condition.value.u8 == 1) {
        last_value = evaluate_node(scope, data->block);
        condition = evaluate_node(scope, data->expr);
    }

    return last_value;
}



Value evaluate_block(Scope* scope, Node node) {
    NodeBlockData* data = (NodeBlockData*) node.pool_ptr;

    Value last_value = (Value) { 0 };

    for (int32_t i = 0; i < data->count; ++i) {
        last_value = evaluate_node(scope, data->nodes[i]);
    }

    return last_value;
}



Value evaluate_bin_expr(Scope* scope, Node node) {
    NodeBinExprData* data = (NodeBinExprData*) node.pool_ptr;

    Value left_value = evaluate_node(scope, data->left);
    Value right_value = evaluate_node(scope, data->right);

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
            return (Value) { 0 };
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

    return result_value;
}

Value evaluate_unary_expr(Scope* scope, Node node) {
    NodeUnaryExprData* data = (NodeUnaryExprData*) node.pool_ptr;

    Value expr_value = evaluate_node(scope, data->expr);

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

    return result_value;
}

Value evaluate_assign_expr(Scope* scope, Node node) {
    NodeAssignExprData* data = (NodeAssignExprData*) node.pool_ptr;

    Value var_value = evaluate_node(scope, data->ident);
    Value assign_value = evaluate_node(scope, data->value);

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
            return (Value) { 0 };
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

    return result_value;
}

Value evaluate_node(Scope* scope, Node node) {
    switch (node.type) {
        case NT_INTEGER_LIT: {
            NodeLiteralData* data = (NodeLiteralData*) node.pool_ptr;
            Value value = (Value) { 0 };
            value.type = VT_INT32;
            value.value.i32 = atoi(data->value);

            return value;
        } break;

        case NT_FLOAT_LIT: {
            NodeLiteralData* data = (NodeLiteralData*) node.pool_ptr;
            Value value = (Value) { 0 };
            value.type = VT_FLOAT32;
            value.value.f32 = atof(data->value);

            return value;
        } break;

        case NT_IDENT_LIT: {
            NodeLiteralData* data = (NodeLiteralData*) node.pool_ptr;
            Value value = scope_get_var(scope, data->value);
            return value;
        } break;


        case NT_RETURN_STAT:
            return evaluate_return_stat(scope, node);

        case NT_VAR_STAT:
            return evaluate_var_stat(scope, node);

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
            return (Value) { 0 };
    }
}



Interpreter create_interpreter(Node ast) {
    Interpreter interp = (Interpreter) {
        .ast = ast,
        .scope = create_scope(),
    };

    return interp;
}

void free_interpreter(Interpreter* interp) {
    free_scope(&interp->scope);
}

void run_interpreter(Interpreter* interp) {
    NodeBlockData* data = (NodeBlockData*) interp->ast.pool_ptr;

    for (int32_t i = 0; i < data->count; ++i) {
        Value value = evaluate_node(&interp->scope, data->nodes[i]);

        printf("Last evaluation: %s: ", ValueTypeNames[value.type]);
        switch (value.type) {
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
                printf("none\n");
                break;
        }
    }
}
