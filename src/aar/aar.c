#include "aar/aar.h"
#include "lexer/token.h"
#include "parser/parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

AARNode* aar_new_node(AARNode node) {
    AARNode* allocation = malloc(1 * sizeof(AARNode));
    *allocation = node;
    return allocation;
}

AARNode aar_parse_node(AARParser* parser, const Node* node);

AARNode aar_parse_block(AARParser* parser, const Node* node) {
    const NBlock* data = &node->data.block;

    // TODO: Add stack frame

    char* label_name = calloc(32, sizeof(char));
    sprintf(label_name, "L%d", parser->label_count++);

    AARNode label_stat = (AARNode) {
        .type = AAR_NT_LABEL_STAT,
        .data.label_stat = (AARNodeLabel) {
            .name = label_name,
        }
    };

    parser->result.data.program.nodes[parser->result.data.program.count++] = label_stat;

    for (int32_t i = 0; i < data->nodes.count; ++i) {
        aar_parse_node(parser, &data->nodes.nodes[i]);
    }

    return (AARNode) { 0 };
}

AARNode aar_parse_bin_expr(AARParser* parser, const Node* node) {
    const NBinExpr* data = &node->data.bin_expr;

    AARNode dst = aar_parse_node(parser, data->left);
    AARNode src = aar_parse_node(parser, data->right);

    AARNode result = (AARNode) { 0 };

    switch (data->op) {
        case TT_PLUS:
            result.type = AAR_NT_ADD_STAT;
            result.data.add_stat = (AARNodeAdd) {
                .src = aar_new_node(src),
                .dst = aar_new_node(dst),
            };
            break;

        case TT_MINUS:
            result.type = AAR_NT_SUB_STAT;
            result.data.sub_stat = (AARNodeSub) {
                .src = aar_new_node(src),
                .dst = aar_new_node(dst),
            };
            break;

        case TT_ASTERISK:
            result.type = AAR_NT_MUL_STAT;
            result.data.mul_stat = (AARNodeMul) {
                .src = aar_new_node(src),
                .dst = aar_new_node(dst),
            };
            break;

        case TT_SLASH:
            result.type = AAR_NT_DIV_STAT;
            result.data.div_stat = (AARNodeDiv) {
                .src = aar_new_node(src),
                .dst = aar_new_node(dst),
            };
            break;

        default:
            assert(false);
            break;
    }


    parser->result.data.program.nodes[parser->result.data.program.count++] = result;

    AARNode register_result = (AARNode) {
        .type = AAR_NT_REG_LIT,
        .data.reg_lit.reg = AARREG_EAX,
    };

    parser->i32_reg -= 1;

    switch (parser->i32_reg) {
        case 0: register_result.data.reg_lit.reg = AARREG_EAX; break;
        case 1: register_result.data.reg_lit.reg = AARREG_EBX; break;
        case 2: register_result.data.reg_lit.reg = AARREG_ECX; break;
        case 3: register_result.data.reg_lit.reg = AARREG_EDX; break;
        default: assert(false); break;
    }

    return register_result;
}

AARNode aar_parse_int_lit(AARParser* parser, const Node* node) {
    const NIntLit* data = &node->data.int_lit;

    AARNode int_lit = (AARNode) {
        .type = AAR_NT_INT_LIT,
        .data.int_lit = (AARNodeInt) {
            .value = data->value,
            .size  = data->size,
        },
    };

    AARNode reg_lit = (AARNode) {
        .type = AAR_NT_REG_LIT,
        .data.reg_lit = (AARNodeReg) {
            .reg = AARREG_EAX,
        },
    };

    parser->i32_reg += 1;

    switch (parser->i32_reg) {
        case 0: reg_lit.data.reg_lit.reg = AARREG_EAX; break;
        case 1: reg_lit.data.reg_lit.reg = AARREG_EBX; break;
        case 2: reg_lit.data.reg_lit.reg = AARREG_ECX; break;
        case 3: reg_lit.data.reg_lit.reg = AARREG_EDX; break;
        default: assert(false); break;
    }

    AARNode mov_stat = (AARNode) {
        .type = AAR_NT_MOV_STAT,
        .data.mov_stat = (AARNodeMov) {
            .dst = aar_new_node(reg_lit),
            .src = aar_new_node(int_lit),
        },
    };

    parser->result.data.program.nodes[parser->result.data.program.count++] = mov_stat;

    return reg_lit;
}

AARNode aar_parse_node(AARParser* parser, const Node* node) {
    switch (node->type) {
        case NT_BLOCK:       return aar_parse_block(parser, node);
        case NT_BIN_EXPR:    return aar_parse_bin_expr(parser, node);
        case NT_INTEGER_LIT: return aar_parse_int_lit(parser, node);
        default: assert(false); break;
    }
}



AARParser create_aar_parser(Node* ast) {
    AARParser parser = (AARParser) { 0 };
    parser.ast = ast;
    parser.i32_reg = -1;

    return parser;
}

void free_aar_parser(AARParser* p) {
    return;
}

void aar_parser_parse(AARParser* p) {
    AARNode* aar_node = &p->result;

    *aar_node = (AARNode) { 0 };
    aar_node->type = AAR_NT_PROGRAM;

    AARNodeProgram* aar_data = &aar_node->data.program;

    aar_data->nodes = calloc(512, sizeof(AARNode));
    aar_data->count = 0;

    const NProgram* ast_data = &p->ast->data.program;

    for (int32_t i = 0; i < ast_data->nodes.count; ++i) {
        aar_parse_node(p, &ast_data->nodes.nodes[i]);
        // AARNode node = aar_parse_node(p, &ast_data->nodes.nodes[i]);

        // if (node.type != AAR_NT_NONE) {
        //     aar_data->nodes[aar_data->count++] = node;
        // }
    }
}
