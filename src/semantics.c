#include "value.h"
#include "parser.h"
#include <assert.h>
#include <semantics.h>
#include <stdio.h>
#include <string.h>

ValueType process_node(Semantics* s, Node* node);

ValueType process_integer_lit(Semantics* s, Node* node) {
    return VT_INT32;
}

ValueType process_float_lit(Semantics* s, Node* node) {
    return VT_FLOAT32;
}

ValueType process_ident_lit(Semantics* s, Node* node) {
    return VT_NONE;
}

ValueType process_return_stat(Semantics* s, Node* node) {
    NRetStat ret_stat = node->data.ret_stat;

    if (ret_stat.expr == NULL) {
        // ret_stat.return_type = VT_NONE;
        return VT_NONE;
    }

    // data->return_type = process_node(s, data->value);
    // return data->return_type;
    return process_node(s, ret_stat.expr);
}

ValueType process_break_stat(Semantics* s, Node* node) {
    return VT_NONE;
}

ValueType process_continue_stat(Semantics* s, Node* node) {
    return VT_NONE;
}

ValueType process_var_stat(Semantics* s, Node* node) {
    NVarStat var_stat = node->data.var_stat;

    char* type_name = var_stat.ident->data.ident_lit.value;

    ValueType variable_type = VT_NONE;

    if (strcmp(type_name, "i8") == 0) {
        variable_type = VT_INT8;
    } else if (strcmp(type_name, "i16") == 0) {
        variable_type = VT_INT16;
    } else if (strcmp(type_name, "i32") == 0) {
        variable_type = VT_INT32;
    } else if (strcmp(type_name, "i64") == 0) {
        variable_type = VT_INT64;
    } else if (strcmp(type_name, "u8") == 0) {
        variable_type = VT_UINT8;
    } else if (strcmp(type_name, "u16") == 0) {
        variable_type = VT_UINT16;
    } else if (strcmp(type_name, "u32") == 0) {
        variable_type = VT_UINT32;
    } else if (strcmp(type_name, "u64") == 0) {
        variable_type = VT_UINT64;
    } else if (strcmp(type_name, "f32") == 0) {
        variable_type = VT_FLOAT32;
    } else if (strcmp(type_name, "f64") == 0) {
        variable_type = VT_FLOAT64;
    }

    // ValueType value_type = process_node(s, data->value);

    // printf("%s\n", ValueTypeNames[variable_type]);
    // data->return_type = variable_type;

    return variable_type;
}

ValueType process_fn_stat(Semantics* s, Node* node) {
    NFuncStat func_stat = node->data.func_stat;

    // char* type_name = ((NodeLiteralData*)(data->type.pool_ptr))->value;

    process_node(s, func_stat.body);

    return VT_NONE;
}


ValueType process_if_stat(Semantics* s, Node* node) {
    NIfStat if_stat = node->data.if_stat;

    process_node(s, if_stat.condition);
    process_node(s, if_stat.body);

    if (if_stat.alternate != NULL) {
        process_node(s, if_stat.alternate);
    }

    return VT_NONE;
}

ValueType process_while_stat(Semantics* s, Node* node) {
    NWhileStat while_stat = node->data.while_stat;

    process_node(s, while_stat.condition);
    process_node(s, while_stat.body);

    return VT_NONE;
}

ValueType process_block(Semantics* s, Node* node) {
    NBlock block = node->data.block;

    ValueType last_type = VT_NONE;

    for (int32_t i = 0; i < block.nodes.count; ++i) {
        last_type = process_node(s, &block.nodes.nodes[i]);
    }

    return last_type;
}

ValueType process_bin_expr(Semantics* s, Node* node) {
    NBinExpr bin_expr = node->data.bin_expr;

    // int + int = int
    // float + float = float
    // int + float = int
    // float + int = float

    // TODO: Implement auto-casting

    Node* left = bin_expr.left;
    ValueType left_type = process_node(s, left);

    Node* right = bin_expr.right;
    process_node(s, right);
    // ValueType right_type = process_node(s, right);

    // if (left_type != right_type) {
    //     sprintf(s->errmsg, "Cannot do an arithmetical expression with %s and %s", ValueTypeNames[left_type], ValueTypeNames[right_type]);
    //     s->error = true;

    //     return VT_NONE;
    // }

    // data->return_type = left_type;

    return left_type;
}

ValueType process_update_expr(Semantics* s, Node* node) {
    NUpdateExpr update_expr = node->data.update_expr;
    // data->return_type = process_node(s, data->expr);

    // return data->return_type;
    return process_node(s, update_expr.expr);
}

ValueType process_assign_expr(Semantics* s, Node* node) {
    NAssignExpr assign_expr = node->data.assign_expr;
    // data->return_type = process_node(s, data->value);

    // return data->return_type;
    return process_node(s, assign_expr.value);
}

ValueType process_unary_expr(Semantics* s, Node* node) {
    NUnExpr un_expr = node->data.un_expr;
    // data->return_type = process_node(s, data->expr);

    // return data->return_type;
    return process_node(s, un_expr.expr);
}

ValueType process_call_expr(Semantics* s, Node* node) {
    return VT_NONE;
}

ValueType process_cast_expr(Semantics* s, Node* node) {
    return get_value_type_from_string(node->data.cast_expr.type->data.ident_lit.value);
}

ValueType process_node(Semantics* s, Node* node) {
    switch (node->type) {
        case NT_INTEGER_LIT:
            return process_integer_lit(s, node);

        case NT_FLOAT_LIT:
            return process_float_lit(s, node);

        case NT_IDENT_LIT:
            return process_ident_lit(s, node);


        case NT_RETURN_STAT:
            return process_return_stat(s, node);

        case NT_BREAK_STAT:
            return process_break_stat(s, node);

        case NT_CONTINUE_STAT:
            return process_continue_stat(s, node);

        case NT_VAR_STAT:
            return process_var_stat(s, node);

        case NT_FUNC_STAT:
            return process_fn_stat(s, node);

        case NT_IF_STAT:
            return process_if_stat(s, node);

        case NT_WHILE_STAT:
            return process_while_stat(s, node);


        case NT_BLOCK:
            return process_block(s, node);


        case NT_BIN_EXPR:
            return process_bin_expr(s, node);

        case NT_UN_EXPR:
            return process_unary_expr(s, node);

        case NT_UPDATE_EXPR:
            return process_update_expr(s, node);

        case NT_ASSIGN_EXPR:
            return process_assign_expr(s, node);

        case NT_CALL_EXPR:
            return process_call_expr(s, node);

        case NT_CAST_EXPR:
            return process_cast_expr(s, node);

        default:
            printf("Unhandled semantics node type: %s\n", NodeTypeNames[node->type]);
            assert(false);
            return VT_NONE;
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

Semantics create_semantics() {
    return (Semantics) {
        .errmsg = { 0 },
        .error = false,
    };
}

void process_semantics(Semantics* s, Node* ast) {
    NProgram program = ast->data.program;

    for (int32_t i = 0; i < program.nodes.count; ++i) {
        Node* node = &program.nodes.nodes[i];
        process_node(s, node);
    }
}
