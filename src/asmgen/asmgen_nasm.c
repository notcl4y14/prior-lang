#include "aar/aar.h"
#include "asmgen/asmgen.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

char* eval(AARNode* node);

char* eval_mov_stat(AARNode* node) {
    AARNodeMov* data = &node->data.mov_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* src = eval(data->src);
    char* dst = eval(data->dst);

    sprintf(strbuf, "mov %s, %s", dst, src);

    free(dst);
    free(src);

    return strbuf;
}

char* eval_add_stat(AARNode* node) {
    AARNodeAdd* data = &node->data.add_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* src = eval(data->src);
    char* dst = eval(data->dst);

    sprintf(strbuf, "add %s, %s", dst, src);

    free(dst);
    free(src);

    return strbuf;
}

char* eval_sub_stat(AARNode* node) {
    AARNodeSub* data = &node->data.sub_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* src = eval(data->src);
    char* dst = eval(data->dst);

    sprintf(strbuf, "sub %s, %s", dst, src);

    free(dst);
    free(src);

    return strbuf;
}

char* eval_mul_stat(AARNode* node) {
    AARNodeAdd* data = &node->data.add_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* src = eval(data->src);
    char* dst = eval(data->dst);

    sprintf(strbuf, "mul %s, %s", dst, src);

    free(dst);
    free(src);

    return strbuf;
}

char* eval_div_stat(AARNode* node) {
    AARNodeDiv* data = &node->data.div_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* src = eval(data->src);
    char* dst = eval(data->dst);

    sprintf(strbuf, "div %s, %s", dst, src);

    free(dst);
    free(src);

    return strbuf;
}

char* eval_reg_lit(AARNode* node) {
    AARNodeReg* data = &node->data.reg_lit;

    char* strbuf = malloc(8 * sizeof(char));

    switch (data->reg) {
        case AARREG_EAX: sprintf(strbuf, "eax"); break;
        case AARREG_EBX: sprintf(strbuf, "ebx"); break;
        case AARREG_ECX: sprintf(strbuf, "ecx"); break;
        case AARREG_EDX: sprintf(strbuf, "edx"); break;

        case AARREG_RAX: sprintf(strbuf, "rax"); break;
        case AARREG_RBX: sprintf(strbuf, "rbx"); break;
        case AARREG_RCX: sprintf(strbuf, "rcx"); break;
        case AARREG_RDX: sprintf(strbuf, "rdx"); break;

        default: assert(false); break;
    }

    return strbuf;
}

char* eval_int_lit(AARNode* node) {
    AARNodeInt* data = &node->data.int_lit;

    char* strbuf = calloc(data->size + 1, sizeof(char));
    // *strbuf = *data->value;
    memcpy(strbuf, data->value, data->size);

    // printf("%s -> %s\n", data->value, strbuf);

    return strbuf;
}

char* eval(AARNode* node) {
    switch (node->type) {
        case AAR_NT_MOV_STAT: return eval_mov_stat(node);
        case AAR_NT_ADD_STAT: return eval_add_stat(node);
        case AAR_NT_SUB_STAT: return eval_sub_stat(node);
        case AAR_NT_MUL_STAT: return eval_mul_stat(node);
        case AAR_NT_DIV_STAT: return eval_div_stat(node);
        case AAR_NT_REG_LIT:  return eval_reg_lit(node);
        case AAR_NT_INT_LIT:  return eval_int_lit(node);
        default: assert(false); break;
    }
}

void asm_gen_nasm(AsmGen* asm_gen) {
    asm_gen_push(asm_gen, "global _main\n_main:\n");

    const AARNode*        aar_node = asm_gen->aar;
    const AARNodeProgram* aar_data = &aar_node->data.program;

    for (int32_t i = 0; i < aar_data->count; ++i) {
        AARNode* node = &aar_data->nodes[i];
        char* strbuf = eval(node);

        asm_gen_push(asm_gen, "  "); // indentation
        asm_gen_push(asm_gen, strbuf); // instruction
        asm_gen_push(asm_gen, "\n"); // newline

        free(strbuf);
    }
}
