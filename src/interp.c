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



EvalResult evaluate_node(Interpreter* interp, Scope* scope, Node* node);

EvalResult evaluate_return_stat(Interpreter* interp, Scope* scope, Node* node) {
    NRetStat ret_stat = node->data.ret_stat;
    Value value = (Value) { 0 };

    if (ret_stat.expr->type != NT_NONE) {
        value = evaluate_node(interp, scope, ret_stat.expr).value;
        // value = cast_value(value, data->return_type);
    }

    return (EvalResult) {
        .value = value,
        .break_type = EBT_RETURN,
    };
}

EvalResult evaluate_break_stat(Interpreter* interp, Scope* scope, Node* node) {
    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_BREAK,
    };
}

EvalResult evaluate_continue_stat(Interpreter* interp, Scope* scope, Node* node) {
    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_CONTINUE,
    };
}

EvalResult evaluate_var_stat(Interpreter* interp, Scope* scope, Node* node) {
    NVarStat var_stat = node->data.var_stat;

    char* name = var_stat.ident->data.ident_lit.value;

    scope_declare_var(scope, name);

    if (var_stat.value->type != NT_NONE) {
        Value value = evaluate_node(interp, scope, var_stat.value).value;
        // value = cast_value(value, var_stat.return_type);

        scope_define_var(scope, name, value);
    }

    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_fn_stat(Interpreter* interp, Scope* scope, Node* node) {
    NFuncStat func_stat = node->data.func_stat;

    char* name = func_stat.ident->data.ident_lit.value;
    char* type = func_stat.type->data.ident_lit.value;

    scope_declare_var(scope, name);

    ValueFunction fn_value = (ValueFunction) {
        .params = { {} },
        .node = func_stat.body,
        .return_type = get_value_type_from_string(type),
    };

    NodeArr* params = &func_stat.params;

    for (int32_t i = 0; i < params->count; ++i) {
        Node* param = &params->nodes[i];

        char* param_name = param->data.parameter.ident->data.ident_lit.value;
        // TODO: Handle array types, but when custom types are implemented
        ValueType param_type = get_value_type_from_string(param->data.parameter.type->data.ident_lit.value);

        fn_value.params[i] = (ValueFunctionParam) {
            .name = param_name,
            .type = param_type,
        };
    }

    // TODO: Create a function table or something to store these
    ValueFunction* fn_ptr = malloc(sizeof(ValueFunction));
    *fn_ptr = fn_value;

    // TODO: Improve pointer handling here
    scope_define_var(scope, name, (Value) { .type = VT_UINT64, .value.u64 = (uint64_t) fn_ptr });

    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_if_stat(Interpreter* interp, Scope* scope, Node* node) {
    NIfStat if_stat = node->data.if_stat;

    Value condition = evaluate_node(interp, scope, if_stat.condition).value;

    if (condition.value.u8 == 1) {
        return evaluate_node(interp, scope, if_stat.body);
    } else {
        if (if_stat.alternate != NULL) {
            return evaluate_node(interp, scope, if_stat.alternate);
        }
    }

    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_while_stat(Interpreter* interp, Scope* scope, Node* node) {
    NWhileStat while_stat = node->data.while_stat;

    EvalResult last_result = (EvalResult) { 0 };
    Value condition = evaluate_node(interp, scope, while_stat.condition).value;

    while (condition.value.u8 == 1) {
        EvalResult next_result = evaluate_node(interp, scope, while_stat.body);

        if (next_result.break_type == EBT_RETURN) {
            last_result = next_result;
            break;
        } else if (next_result.break_type == EBT_BREAK) {
            break;
        } else if (next_result.break_type == EBT_CONTINUE) {
            condition = evaluate_node(interp, scope, while_stat.condition).value;
            continue;
        }

        last_result = next_result;
        condition = evaluate_node(interp, scope, while_stat.condition).value;
    }

    return last_result;
}



EvalResult evaluate_block(Interpreter* interp, Scope* scope, Node* node) {
    NBlock block = node->data.block;

    Scope sub_scope = create_scope(scope);

    EvalResult last_result = (EvalResult) { 0 };

    for (int32_t i = 0; i < block.nodes.count; ++i) {
        last_result = evaluate_node(interp, &sub_scope, &block.nodes.nodes[i]);

        if (last_result.break_type == EBT_RETURN) {
            break;
        }
    }

    free_scope(&sub_scope);

    return last_result;
}



EvalResult evaluate_bin_expr(Interpreter* interp, Scope* scope, Node* node) {
    NBinExpr bin_expr = node->data.bin_expr;

    Value left_value = evaluate_node(interp, scope, bin_expr.left).value;
    Value right_value = evaluate_node(interp, scope, bin_expr.right).value;

    /* Auto-casting */
    if (left_value.type != right_value.type) {
        right_value = cast_value(right_value, left_value.type);
    }

    Value result_value = (Value) { 0 };
    result_value.type = left_value.type;

    // TODO: Replace the switch wall
    if (bin_expr.op == TT_PLUS) {
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
    } else if (bin_expr.op == TT_MINUS) {
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
    } else if (bin_expr.op == TT_ASTERISK) {
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
    } else if (bin_expr.op == TT_SLASH) {
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
    } else if (bin_expr.op == TT_EQUALS) {
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
    } else if (bin_expr.op == TT_NOT_EQUALS) {
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
    } else if (bin_expr.op == TT_LESS) {
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
    } else if (bin_expr.op == TT_GREATER) {
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
    } else if (bin_expr.op == TT_LESS_EQUALS) {
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
    } else if (bin_expr.op == TT_GREATER_EQUALS) {
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

EvalResult evaluate_unary_expr(Interpreter* interp, Scope* scope, Node* node) {
    NUnExpr un_expr = node->data.un_expr;

    Value expr_value = evaluate_node(interp, scope, un_expr.expr).value;

    Value result_value = (Value) { 0 };
    result_value.type = expr_value.type;

    if (un_expr.op == TT_MINUS) {
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

EvalResult evaluate_assign_expr(Interpreter* interp, Scope* scope, Node* node) {
    NAssignExpr assign_expr = node->data.assign_expr;

    Value var_value = evaluate_node(interp, scope, assign_expr.ident).value;
    Value assign_value = evaluate_node(interp, scope, assign_expr.value).value;

    /*  Auto-casting */
    if (var_value.type != assign_value.type) {
        assign_value = cast_value(assign_value, var_value.type);
    }

    Value result_value = (Value) { 0 };
    result_value.type = var_value.type;

    // TODO: Replace the switch wall
    if (assign_expr.op == TT_EQUAL) {
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
    } else if (assign_expr.op == TT_PLUS_EQUAL) {
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
    } else if (assign_expr.op == TT_MINUS_EQUAL) {
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
    } else if (assign_expr.op == TT_TIMES_EQUAL) {
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
    } else if (assign_expr.op == TT_DIVIDE_EQUAL) {
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

    scope_define_var(scope, assign_expr.ident->data.ident_lit.value, result_value);

    return (EvalResult) {
        .value = result_value,
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_call_expr(Interpreter* interp, Scope* scope, Node* node) {
    NCallExpr call_expr = node->data.call_expr;

    // TODO: Member expr
    if (call_expr.member->type == NT_MEMBER_EXPR) {
        printf("Functions with idents only yet\n");
        exit(1);
    }

    Value fn_ptr = evaluate_node(interp, scope, call_expr.member).value;
    ValueFunction* fn_value = (ValueFunction*) fn_ptr.value.u64;

    EvalResult result = (EvalResult) { 0 };

    {
        NodeArr arg_array = call_expr.args;
        NBlock fn_block = fn_value->node->data.block;

        ValueFunctionParam* param_list = (ValueFunctionParam*) fn_value->params;

        Scope sub_scope = create_scope(&interp->scope);

        // Defining parameter values with arguments
        for (int32_t i = 0; i < arg_array.count; ++i) {
            NArgument argument = arg_array.nodes[i].data.argument;
            Value arg_value = evaluate_node(interp, &sub_scope, argument.expr).value;
            ValueFunctionParam param = param_list[i];

            scope_declare_var(&sub_scope, param.name);
            scope_define_var(&sub_scope, param.name, arg_value);
        }

        EvalResult last_result = (EvalResult) { 0 };

        for (int32_t i = 0; i < fn_block.nodes.count; ++i) {
            last_result = evaluate_node(interp, &sub_scope, &fn_block.nodes.nodes[i]);

            if (last_result.break_type == EBT_RETURN) {
                break;
            }
        }

        free_scope(&sub_scope);

        result = last_result;
    }

    return result;
}

EvalResult evaluate_cast_expr(Interpreter* interp, Scope* scope, Node* node) {
    NCastExpr cast_expr = node->data.cast_expr;

    // TODO: Member expr
    if (cast_expr.type->type == NT_MEMBER_EXPR) {
        printf("Casts with idents only yet\n");
        exit(1);
    }

    ValueType cast_type = get_value_type_from_string(cast_expr.type->data.ident_lit.value);

    Value value = evaluate_node(interp, scope, cast_expr.expr).value;
    Value result = cast_value(value, cast_type);

    return (EvalResult) { .value = result, .break_type = EBT_NONE };
}

EvalResult evaluate_struct_stat(Interpreter* interp, Scope* scope, Node* node) {
    /***
     * We don't need to evaluate struct statements, they are already
     * handled by the Semantics Processing. What we need to evaluate
     * is struct instance creations.
     */
    // const uint32_t index = interp->scope.structcount++;
    // NodeArr fields = node->data.struct_stat.fields;

    // Struct struct_;
    // struct_.count = fields.count;
    // struct_.entries = malloc(sizeof(*struct_.entries) * struct_.count);
    // struct_.values = malloc(sizeof(*struct_.values) * struct_.count);

    // for (size_t i = 0; i < fields.count; i++) {
    //     struct_.entries[i] = str_alloc_copy(fields.nodes[i].data.field.ident->data.ident_lit.value);
    //     // TODO: add the values
    // }
    return (EvalResult) { 0 };
}

EvalResult evaluate_node(Interpreter* interp, Scope* scope, Node* node) {
    switch (node->type) {
        case NT_INTEGER_LIT: {
            NIntLit int_lit = node->data.int_lit;

            Value value = (Value) { 0 };
            value.type = VT_INT32;
            value.value.i32 = atoi(int_lit.value);

            return (EvalResult) { .value = value, .break_type = EBT_NONE };
        } break;

        case NT_FLOAT_LIT: {
            NFloatLit float_lit = node->data.float_lit;

            Value value = (Value) { 0 };
            value.type = VT_FLOAT32;
            value.value.f32 = atof(float_lit.value);

            return (EvalResult) { .value = value, .break_type = EBT_NONE };
        } break;

        case NT_IDENT_LIT: {
            NIdentLit ident_lit = node->data.ident_lit;

            Value value = scope_get_var(scope, ident_lit.value);

            return (EvalResult) { .value = value, .break_type = EBT_NONE };
        } break;

        case NT_RETURN_STAT:   return evaluate_return_stat(interp, scope, node);
        case NT_BREAK_STAT:    return evaluate_break_stat(interp, scope, node);
        case NT_CONTINUE_STAT: return evaluate_continue_stat(interp, scope, node);
        case NT_VAR_STAT:      return evaluate_var_stat(interp, scope, node);
        case NT_FUNC_STAT:     return evaluate_fn_stat(interp, scope, node);
        case NT_IF_STAT:       return evaluate_if_stat(interp, scope, node);
        case NT_WHILE_STAT:    return evaluate_while_stat(interp, scope, node);

        case NT_BLOCK: return evaluate_block(interp, scope, node);

        case NT_BIN_EXPR:    return evaluate_bin_expr(interp, scope, node);
        case NT_UN_EXPR:     return evaluate_unary_expr(interp, scope, node);
        case NT_ASSIGN_EXPR: return evaluate_assign_expr(interp, scope, node);
        case NT_CALL_EXPR:   return evaluate_call_expr(interp, scope, node);
        case NT_CAST_EXPR:   return evaluate_cast_expr(interp, scope, node);
        case NT_STRUCT_STAT: return evaluate_struct_stat(interp, scope, node);

        default:
            printf("Node type %s not handled in evaluation switch\n", NodeTypeNames[node->type]);
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
    NProgram program = interp->ast.data.program;

    for (int32_t i = 0; i < program.nodes.count; ++i) {
        evaluate_node(interp, &interp->scope, &program.nodes.nodes[i]);
    }

    Value main_fn_ptr = scope_get_var(&interp->scope, "main");
    ValueFunction* main_fn_value = (ValueFunction*) main_fn_ptr.value.u64;
    NBlock fn_block = main_fn_value->node->data.block;

    for (int32_t i = 0; i < fn_block.nodes.count; ++i) {
        EvalResult result = evaluate_node(interp, &interp->scope, &fn_block.nodes.nodes[i]);
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
