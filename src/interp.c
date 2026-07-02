#include "scope.h"
#include "type.h"
#include "value_types.h"
#include <interp.h>
#include <mem.h>
#include <parser.h>
#include <stddef.h>
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

    /* Evaluating defers before returning */
    for (int32_t i = scope->defer_count - 1; i >= 0; --i) {
        evaluate_node(interp, scope, &scope->defers[i]);
    }
    scope->is_deferred = true;

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
    /* Evaluating defers before returning */
    for (int32_t i = scope->defer_count - 1; i >= 0; --i) {
        evaluate_node(interp, scope, &scope->defers[i]);
    }
    scope->is_deferred = true;

    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_BREAK,
    };
}

EvalResult evaluate_continue_stat(Interpreter* interp, Scope* scope, Node* node) {
    /* Evaluating defers before returning */
    for (int32_t i = scope->defer_count - 1; i >= 0; --i) {
        evaluate_node(interp, scope, &scope->defers[i]);
    }
    scope->is_deferred = true;

    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_CONTINUE,
    };
}

EvalResult evaluate_defer_stat(Interpreter* interp, Scope* scope, Node* node) {
    NDeferStat defer_stat = node->data.defer_stat;

    scope_add_defer(scope, *defer_stat.expr);

    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_NONE,
    };
}

/***
 * Separator
 */

/***
 * Creates interface variables for the compound literal
 * so that the struct instantiation would not have to rely
 * on the AST.
 *
 * NCompoundLit* compound; // The compound literal data we're initializing off of
 * const char*** comp_k;   // Pointer to compound keys array (identifiers)
 * Node*** comp_v;         // Pointer to compound values array (ndoes)
 * size_t* comp_c;         // Pointer to compound size variable
 */
void init_compound_vars(NCompoundLit* compound, const char*** comp_k, Node*** comp_v, size_t* comp_c) {
    size_t compound_entry_amount = compound->values.count;
    *comp_c = compound_entry_amount;

    *comp_k = calloc(compound_entry_amount, sizeof(char**));
    *comp_v = calloc(compound_entry_amount, sizeof(Node*));

    for (int32_t i = 0; i < compound_entry_amount; ++i) {
        const Node* assignment_node = &compound->values.nodes[i];

        (*comp_k)[i] = assignment_node->data.assign_expr.ident->data.ident_lit.value;
        (*comp_v)[i] = assignment_node->data.assign_expr.value;

        // printf("%s\n", NodeTypeNames[(*comp_v)[i]->type]);
    }
}

Value instantiate_struct
   (Type st, TypeTable* tt, Scope* scope, Interpreter* interp,
    const char** comp_k, Node** comp_v, size_t comp_c)
{
    /***
     * st - Struct Type. A.K.A: Type definition of the Struct.
     * sv - Struct Value. A.K.A: Runtime value of the struct.
     */
    Value sv = (Value) { .type = VT_STRUCT };

    /* Initializing struct value data */
    Struct* sv_data = &sv.value.struct_;
    *sv_data = create_struct_value_data(st.data.data_struct.count);

    /* Inserting compound values into the fields */
    for (int32_t i = 0; i < st.data.data_struct.count; ++i) {
        Type field_type = type_table_get_type(tt, st.data.data_struct.fields_types[i]);
        ValueType field_value_type = get_typedef_value_type(field_type);
        char* st_field_ident = st.data.data_struct.fields_names[i];

        sv_data->fields[i] = st_field_ident;
        sv_data->values[i].type = field_value_type;

        const char* compound_field_ident = NULL;
        Value       compound_field_value = (Value) { 0 };

        /* Iterating through compound fields to search for the matching field */
        for (int32_t j = 0; j < comp_c; ++j) {
            /* Getting compound field's identifier */
            compound_field_ident = comp_k[j];

            /* Checking if the compound field matches with the struct field */
            if (strcmp(st_field_ident, compound_field_ident) == 0) {
                /* Evaluating the compound value */
                compound_field_value = evaluate_node(interp, scope, comp_v[j]).value;

                break;
            }
        }

        /* If the matching struct field has NOT been found in the compound */
        if (compound_field_value.type == VT_NONE) {
            compound_field_value.type = field_value_type;
            value_to_zero(&compound_field_value); /* We zero it out */
        }

        /* Auto-cast */
        if (compound_field_value.type != sv_data->values[i].type) {
            compound_field_value = cast_value(compound_field_value, sv_data->values[i].type);
        }

        /* Assigning the compound field to the struct instance field */
        sv_data->values[i].value = compound_field_value.value;
    }

    return sv;
}

/***
 * Separator
 */

EvalResult evaluate_var_stat(Interpreter* interp, Scope* scope, Node* node) {
    NVarStat var_stat = node->data.var_stat;

    char* ident_name = var_stat.ident->data.ident_lit.value;
    char* type_name = var_stat.type->data.ident_lit.value;

    Type type = type_table_get_type(&interp->scope.type_table, type_name);

    scope_declare_var(scope, ident_name, type);

    /* Yes value, evaluating */
    if (var_stat.value != NULL) {
        Value value = (Value) { 0 };

        switch (type.type) {
            case TYPE_TYPE_STRUCT: {
                assert(var_stat.value->type == NT_COMPOUND_LIT);

                const char** comp_k = NULL;
                Node** comp_v = NULL;
                size_t comp_c = 0;
                init_compound_vars(&var_stat.value->data.compound_lit, &comp_k, &comp_v, &comp_c);

                value = instantiate_struct(type, &interp->scope.type_table, scope, interp, comp_k, comp_v, comp_c);

                free(comp_v);
                comp_v = NULL;

                free(comp_k);
                comp_k = NULL;
            } break;

            default:
                value = evaluate_node(interp, scope, var_stat.value).value;
                break;
        }

        scope_define_var(scope, ident_name, value);
    }
    /* No value, zeroing out */
    else {
        Value value = (Value) { 0 };

        switch (type.type) {
            case TYPE_TYPE_STRUCT:
                /***
                 * A little trick: We're telling the struct instantiator that
                 * the amount of compound entries is zero and it's going to
                 * zero out each entry evaluation.
                 */
                value = instantiate_struct(type, &interp->scope.type_table, scope, interp, NULL, NULL, 0);
                break;

            default:
                value = evaluate_node(interp, scope, var_stat.value).value;
                break;
        }

        scope_define_var(scope, ident_name, value);
    }

    return (EvalResult) {
        .value = (Value) { 0 },
        .break_type = EBT_NONE,
    };
}

EvalResult evaluate_fn_stat(Interpreter* interp, Scope* scope, Node* node) {
    NFuncStat func_stat = node->data.func_stat;

    char* name = func_stat.ident->data.ident_lit.value;
    char* type_name = func_stat.type->data.ident_lit.value;
    Type type = type_table_get_type(&scope->type_table, type_name);

    scope_declare_var(scope, name, type);

    ValueFunction fn_value = (ValueFunction) {
        .params = { {} },
        .node = func_stat.body,
        .return_type = type,
    };

    NodeArr* params = &func_stat.params;

    for (int32_t i = 0; i < params->count; ++i) {
        Node* param = &params->nodes[i];

        char* param_name = param->data.parameter.ident->data.ident_lit.value;
        // TODO: Handle array types
        Type param_type = type_table_get_type(&scope->type_table, param->data.parameter.type->data.ident_lit.value);

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

    /* Evaluating defers */
    if (!sub_scope.is_deferred) {
        for (int32_t i = sub_scope.defer_count - 1; i >= 0; --i) {
            evaluate_node(interp, &sub_scope, &sub_scope.defers[i]);
        }
        sub_scope.is_deferred = true;
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

            scope_declare_var(&sub_scope, param.name, param.type);
            scope_define_var(&sub_scope, param.name, arg_value);
        }

        EvalResult last_result = (EvalResult) { 0 };

        for (int32_t i = 0; i < fn_block.nodes.count; ++i) {
            last_result = evaluate_node(interp, &sub_scope, &fn_block.nodes.nodes[i]);

            if (last_result.break_type == EBT_RETURN) {
                break;
            }
        }

        /* Evaluating defers */
        if (!sub_scope.is_deferred) {
            for (int32_t i = sub_scope.defer_count - 1; i >= 0; --i) {
                evaluate_node(interp, &sub_scope, &sub_scope.defers[i]);
            }
            sub_scope.is_deferred = true;
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
        case NT_DEFER_STAT:    return evaluate_defer_stat(interp, scope, node);
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

    // Scope sub_scope = create_scope(&interp->scope);

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

    /* Evaluating defers */
    // if (!sub_scope.is_deferred) {
    //     for (int32_t i = sub_scope.defer_count - 1; i >= 0; --i) {
    //         evaluate_node(interp, &sub_scope, &sub_scope.defers[i]);
    //     }
    //     sub_scope.is_deferred = true;
    // }

    // free_scope(&sub_scope);
}
