#include "aar.h"
#include "lexer/token.h"
#include "parser/parser.h"
#include <assert.h>
#include <stdlib.h>

AARNode* aar_new_node(AARNode node) {
    AARNode* allocation = malloc(1 * sizeof(AARNode));
    *allocation = node;
    return allocation;
}

AARNode aar_parse_node(const Node* node);

AARNode aar_parse_bin_expr(const Node* node) {
    const NBinExpr* data = &node->data.bin_expr;

    AARNode src = aar_parse_node(data->left);
    AARNode dst = aar_parse_node(data->right);

    AARNode result = (AARNode) { 0 };

    switch (data->op) {
        case TT_PLUS:
            result.type = AAR_NT_ADD_STAT;
            result.data.add = (AARNAdd) {
                .src = aar_new_node(src),
                .dst = aar_new_node(dst),
            };
            break;

        case TT_MINUS:
            result.type = AAR_NT_SUB_STAT;
            result.data.sub = (AARNSub) {
                .src = aar_new_node(src),
                .dst = aar_new_node(dst),
            };
            break;

        case TT_ASTERISK:
            result.type = AAR_NT_MUL_STAT;
            result.data.mul = (AARNMul) {
                .src = aar_new_node(src),
                .dst = aar_new_node(dst),
            };
            break;

        case TT_SLASH:
            result.type = AAR_NT_DIV_STAT;
            result.data.div = (AARNDiv) {
                .src = aar_new_node(src),
                .dst = aar_new_node(dst),
            };
            break;

        default:
            assert(false);
            break;
    }

    return result;
}

AARNode aar_parse_int_lit(const Node* node) {
    const NIntLit* data = &node->data.int_lit;

    AARNode result = (AARNode) {
        .type = AAR_NT_INT_LIT,
        .data.int_ = (AARNInt) {
            .value = data->value,
            .size  = data->size,
        },
    };

    return result;
}

AARNode aar_parse_node(const Node* node) {
    switch (node->type) {
        case NT_BIN_EXPR:    return aar_parse_bin_expr(node);
        case NT_INTEGER_LIT: return aar_parse_int_lit(node);
        default: assert(false); break;
    }
}



AARParser create_aar_parser(Node* ast) {
    AARParser parser = (AARParser) { 0 };
    parser.ast = ast;

    return parser;
}

void free_aar_parser(AARParser* p) {
    return;
}

void aar_parser_parse(AARParser* p) {
    AARNode aar_node = (AARNode) {
        .type = AAR_NT_PROGRAM,
    };

    AARNProgram* aar_data = &aar_node.data.program;

    aar_data->nodes = calloc(512, sizeof(AARNode));
    aar_data->count = 0;

    const NProgram* ast_data = &p->ast->data.program;

    for (int32_t i = 0; i < ast_data->nodes.count; ++i) {
        aar_data->nodes[aar_data->count++] = aar_parse_node(&ast_data->nodes.nodes[i]);
    }

    p->result = aar_node;
}
