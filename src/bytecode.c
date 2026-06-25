#include "lexer.h"
#include "parser.h"
#include "value.h"
#include <assert.h>
#include <bytecode.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* OpCodeNames[] = {
    [OP_RET]  = "RET",
    [OP_CALL] = "CALL",
    [OP_SYS]  = "SYS",
    [OP_PSH]  = "PSH",
    [OP_POP]  = "POP",
    [OP_LOAD] = "LOAD",
    [OP_STOR] = "STOR",
    [OP_NG]   = "NG",
    [OP_ADD]  = "ADD",
    [OP_SUB]  = "SUB",
    [OP_MUL]  = "MUL",
    [OP_DIV]  = "DIV",
    [OP_UMUL] = "UMUL",
    [OP_UDIV] = "UDIV",
    [OP_FADD] = "FADD",
    [OP_FSUB] = "FSUB",
    [OP_FMUL] = "FMUL",
    [OP_FDIV] = "FDIV",
    [OP_AND]  = "AND",
    [OP_OR]   = "OR",
    [OP_XOR]  = "XOR",
    [OP_NOT]  = "NOT",
    [OP_CMP]  = "CMP",
    [OP_JMP]  = "JMP",
    [OP_JEQ]  = "JEQ",
    [OP_JNE]  = "JNE",
    [OP_JLT]  = "JLT",
    [OP_JGT]  = "JGT",
    [OP_JLE]  = "JLE",
    [OP_JGE]  = "JGE",
};



void init_bytecode(Bytecode* bc, size_t size) {
    bc->bytes = calloc(size, sizeof(uint8_t));
    assert(bc->bytes != NULL);

    bc->count = 0;
    bc->capacity = size;
}

void free_bytecode(Bytecode* bc) {
    free(bc->bytes);
    bc->bytes = NULL;
}

void print_bytecode(Bytecode* bc) {
    for (int32_t i = 0; i < bc->count; ++i) {
        OpCode opcode = bc->bytes[i];

        printf("0x%.4X  %s\t", i, OpCodeNames[opcode]);

        switch (opcode) {
            case OP_PSH: {
                uint8_t push_size = bc->bytes[++i];
                printf("%d", push_size);

                for (int32_t j = 0; j < push_size; ++j) {
                    printf(" %.2X", bc->bytes[++i]);
                }
            } break;

            case OP_POP: {
                uint8_t pop_size = bc->bytes[++i];
                printf("%d", pop_size);
            } break;

            case OP_ADD:
                break;

            case OP_SUB:
                break;

            case OP_MUL:
                break;

            case OP_DIV:
                break;

            case OP_UMUL:
                break;

            case OP_UDIV:
                break;

            case OP_FADD:
                break;

            case OP_FSUB:
                break;

            case OP_FMUL:
                break;

            case OP_FDIV:
                break;

            default:
                assert(false);
                break;
        }

        printf("\n");
    }
}



void bytegen_handle_node(Bytegen* bg, Node node);

void bytegen_emit_byte(Bytegen* bg, uint8_t byte) {
    bg->result.bytes[bg->result.count++] = byte;
}

void bytegen_emit_bytes(Bytegen* bg, uint8_t* bytes, size_t size) {
    memcpy(&bg->result.bytes[bg->result.count], bytes, size);
    bg->result.count += size;
}

void bytegen_handle_integer(Bytegen* bg, Node node) {
    NodeLiteralData* data = (NodeLiteralData*) node.pool_ptr;

    uint32_t integer = atoi(data->value);
    uint8_t* bytes = (uint8_t*) &integer;

    bytegen_emit_byte(bg, OP_PSH);
    bytegen_emit_byte(bg, sizeof(uint32_t));
    bytegen_emit_bytes(bg, bytes, sizeof(uint32_t));
}

void bytegen_handle_float(Bytegen* bg, Node node) {
    NodeLiteralData* data = (NodeLiteralData*) node.pool_ptr;

    float value = atof(data->value);
    uint8_t* bytes = (uint8_t*) &value;

    // printf("%f\n", *(float*) bytes);

    bytegen_emit_byte(bg, OP_PSH);
    bytegen_emit_byte(bg, sizeof(uint32_t));
    bytegen_emit_bytes(bg, bytes, sizeof(uint32_t));
}

void bytegen_handle_bin_expr(Bytegen* bg, Node node) {
    NodeBinExprData* data = (NodeBinExprData*) node.pool_ptr;

    TokenType op = data->op;
    Node left = data->left;
    Node right = data->right;

    bytegen_handle_node(bg, right);
    bytegen_handle_node(bg, left);

    if (data->return_type == VT_INT32) {
        if (op == TT_PLUS) {
            bytegen_emit_byte(bg, OP_ADD);
        } else if (op == TT_MINUS) {
            bytegen_emit_byte(bg, OP_SUB);
        } else if (op == TT_ASTERISK) {
            bytegen_emit_byte(bg, OP_MUL);
        } else if (op == TT_SLASH) {
            bytegen_emit_byte(bg, OP_DIV);
        } else if (op == TT_MODULO) {
            bytegen_emit_byte(bg, OP_AND);
        }
    } else if (data->return_type == VT_UINT32) {
        if (op == TT_PLUS) {
            bytegen_emit_byte(bg, OP_ADD);
        } else if (op == TT_MINUS) {
            bytegen_emit_byte(bg, OP_SUB);
        } else if (op == TT_ASTERISK) {
            bytegen_emit_byte(bg, OP_UMUL);
        } else if (op == TT_SLASH) {
            bytegen_emit_byte(bg, OP_UDIV);
        } else if (op == TT_MODULO) {
            bytegen_emit_byte(bg, OP_AND);
        }
    } else if (data->return_type == VT_FLOAT32) {
        if (op == TT_PLUS) {
            bytegen_emit_byte(bg, OP_FADD);
        } else if (op == TT_MINUS) {
            bytegen_emit_byte(bg, OP_FSUB);
        } else if (op == TT_ASTERISK) {
            bytegen_emit_byte(bg, OP_FMUL);
        } else if (op == TT_SLASH) {
            bytegen_emit_byte(bg, OP_FDIV);
        }
    }
}

void bytegen_handle_node(Bytegen* bg, Node node) {
    switch (node.type) {
        case NT_BIN_EXPR:
            bytegen_handle_bin_expr(bg, node);
            break;

        case NT_INTEGER_LIT:
            bytegen_handle_integer(bg, node);
            break;

        case NT_FLOAT_LIT:
            bytegen_handle_float(bg, node);
            break;

        default:
            assert(false);
            break;
    }
}



void init_bytegen(Bytegen* bg, Node ast) {
    bg->ast = ast;
    init_bytecode(&bg->result, 512);
}

Bytecode bytegen_generate(Bytegen* bg) {
    NodeBlockData* program_data = (NodeBlockData*) bg->ast.pool_ptr;

    for (int32_t i = 0; i < program_data->count; ++i) {
        Node node = program_data->nodes[i];
        bytegen_handle_node(bg, node);
    }

    return bg->result;
}
