#include "aar.h"
#include "asmgen.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

char* eval(AARNode* node);

char* eval_mov_stat(AARNode* node) {
    AARNMov* data = &node->data.mov;

    char* strbuf = malloc(256 * sizeof(char));

    char* src = eval(data->src);
    char* dst = eval(data->dst);

    sprintf(strbuf, "mov %s, %s", src, dst);

    free(dst);
    free(src);

    return strbuf;
}

char* eval_add_stat(AARNode* node) {
    AARNAdd* data = &node->data.add;

    char* strbuf = malloc(256 * sizeof(char));

    char* src = eval(data->src);
    char* dst = eval(data->dst);

    sprintf(strbuf, "add %s, %s", src, dst);

    free(dst);
    free(src);

    return strbuf;
}

char* eval_reg_lit(AARNode* node) {
    AARNReg* data = &node->data.reg;

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
    AARNInt* data = &node->data.int_;

    char* strbuf = calloc(data->size + 1, sizeof(char));
    *strbuf = *data->value;

    return strbuf;
}

char* eval(AARNode* node) {
    switch (node->type) {
        case AAR_NT_MOV_STAT: return eval_mov_stat(node);
        case AAR_NT_ADD_STAT: return eval_add_stat(node);
        case AAR_NT_REG_LIT:  return eval_reg_lit(node);
        case AAR_NT_INT_LIT:  return eval_int_lit(node);
        default: assert(false); break;
    }
}

void asm_gen_nasm(AsmGen* asm_gen) {
    asm_gen_push(asm_gen, "global _main\n_main:\n");

    const AARNode*     aar_node = asm_gen->aar;
    const AARNProgram* aar_data = &aar_node->data.program;

    for (int32_t i = 0; i < aar_data->count; ++i) {
        AARNode* node = &aar_data->nodes[i];
        char* strbuf = eval(node);

        asm_gen_push(asm_gen, "  "); // indentation
        asm_gen_push(asm_gen, strbuf); // instruction

        free(strbuf);
    }
}
