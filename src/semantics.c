#include "value.h"
#include <parser.h>
#include <assert.h>
#include <semantics.h>
#include <stdio.h>
#include <string.h>

ValueType process_node(Semantics* s, Node node);

ValueType process_integer_lit(Semantics* s, Node node) {
    return VT_INT32;
}

ValueType process_float_lit(Semantics* s, Node node) {
    return VT_FLOAT32;
}

ValueType process_ident_lit(Semantics* s, Node node) {
    return VT_INT32;
}

ValueType process_var_stat(Semantics* s, Node node) {
    NodeVarStatData* data = (NodeVarStatData*) node.pool_ptr;

    char* type_name = ((NodeLiteralData*)(data->type.pool_ptr))->value;

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

    data->return_type = variable_type;

    return variable_type;
}

ValueType process_bin_expr(Semantics* s, Node node) {
    NodeBinExprData* data = (NodeBinExprData*) node.pool_ptr;

    // int + int = int
    // float + float = float
    // int + float = int
    // float + int = float

    Node left = data->left;
    ValueType left_type = process_node(s, left);

    Node right = data->right;
    ValueType right_type = process_node(s, right);

    if (left_type != right_type) {
        sprintf(s->errmsg, "Cannot do an arithmetical expression with %s and %s", ValueTypeNames[left_type], ValueTypeNames[right_type]);
        s->error = true;

        return VT_NONE;
    }

    data->return_type = left_type;

    return left_type;
}

ValueType process_update_expr(Semantics* s, Node node) {
    NodeUpdateExprData* data = (NodeUpdateExprData*) node.pool_ptr;
    data->return_type = process_node(s, data->expr);

    return data->return_type;
}

ValueType process_unary_expr(Semantics* s, Node node) {
    NodeUnaryExprData* data = (NodeUnaryExprData*) node.pool_ptr;
    data->return_type = process_node(s, data->expr);

    return data->return_type;
}

ValueType process_node(Semantics* s, Node node) {
    switch (node.type) {
        case NT_INTEGER_LIT:
            return process_integer_lit(s, node);

        case NT_FLOAT_LIT:
            return process_float_lit(s, node);

        case NT_IDENT_LIT:
            return process_ident_lit(s, node);

        case NT_VAR_STAT:
            return process_var_stat(s, node);

        case NT_BIN_EXPR:
            return process_bin_expr(s, node);

        case NT_UNARY_EXPR:
            return process_unary_expr(s, node);

        case NT_UPDATE_EXPR:
            return process_update_expr(s, node);

        default:
            printf("Unhandled semantics node type: %s\n", NodeTypeNames[node.type]);
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

void process_semantics(Semantics* s, Node ast) {
    NodeBlockData* program_data = (NodeBlockData*) ast.pool_ptr;

    for (int32_t i = 0; i < program_data->count; ++i) {
        Node node = program_data->nodes[i];
        process_node(s, node);
    }
}
