#include "eval.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

EvalRes evaluate_expr_bin_expr(Scope* scope, Node* node) {
    NBinExpr bin_expr = node->data.bin_expr;

    Value left_value = evaluate_expr_node(scope, bin_expr.left).value;
    Value right_value = evaluate_expr_node(scope, bin_expr.right).value;

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

    return (EvalRes) { result_value, 0 };
}

EvalRes evaluate_expr_unary_expr(Scope* scope, Node* node) {
    NUnExpr un_expr = node->data.un_expr;

    Value expr_value = evaluate_expr_node(scope, un_expr.expr).value;

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

    return (EvalRes) { result_value, 0 };
}

EvalRes evaluate_expr_node(Scope* scope, Node* node) {
    switch (node->type) {
        case NT_INTEGER_LIT: {
            NIntLit int_lit = node->data.int_lit;

            Value value = (Value) { 0 };
            value.type = VT_INT32;
            value.value.i32 = atoi(int_lit.value);

            return (EvalRes) { .value = value, 0 };
        } break;

        case NT_FLOAT_LIT: {
            NFloatLit float_lit = node->data.float_lit;

            Value value = (Value) { 0 };
            value.type = VT_FLOAT32;
            value.value.f32 = atof(float_lit.value);

            return (EvalRes) { .value = value, 0 };
        } break;

        case NT_IDENT_LIT: {
            NIdentLit ident_lit = node->data.ident_lit;

            Value value = scope_get_var(scope, ident_lit.value);

            return (EvalRes) { .value = value, 0 };
        } break;

        case NT_BIN_EXPR: return evaluate_expr_bin_expr(scope, node);
        case NT_UN_EXPR:  return evaluate_expr_unary_expr(scope, node);

        default:
            printf("Node type %s not handled in EXPRESSION evaluation switch\n", NodeTypeNames[node->type]);
            assert(false);
            return (EvalRes) { 0 };
    }
}
