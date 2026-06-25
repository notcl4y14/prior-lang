#include "value.h"
#include <parser.h>
#include <assert.h>
#include <semantics.h>
#include <stdio.h>
#include <string.h>

const char* ValueTypeNames[] = {
    [VT_NONE]    = "(none)",
    [VT_INT32]   = "i32",
    [VT_UINT32]  = "u32",
    [VT_FLOAT32] = "f32",
};

ValueType process_node(Semantics* s, Node node);

ValueType process_integer_lit(Semantics* s, Node node) {
    return VT_INT32;
}

ValueType process_float_lit(Semantics* s, Node node) {
    return VT_FLOAT32;
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
        // printf("Cast the value buddy\n");
        // exit(1);
        sprintf(s->errmsg, "Cannot do an arithmetical expression with %s and %s", ValueTypeNames[left_type], ValueTypeNames[right_type]);
        s->error = true;

        return VT_NONE;
    }

    data->return_type = left_type;

    return left_type;
}

ValueType process_node(Semantics* s, Node node) {
    switch (node.type) {
        case NT_INTEGER_LIT:
            return process_integer_lit(s, node);

        case NT_FLOAT_LIT:
            return process_float_lit(s, node);

        case NT_BIN_EXPR:
            return process_bin_expr(s, node);

        default:
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
