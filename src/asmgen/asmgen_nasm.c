#include "aar/aar.h"
#include "asmgen/asmgen.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

char* eval(AARNode* node);

char* eval_label_stat(AARNode* node) {
    AARNodeLabel* data = &node->data.label_stat;

    char* strbuf = malloc(256 * sizeof(char));

    sprintf(strbuf, "%s:", data->name);

    return strbuf;
}

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

char* eval_cmp_stat(AARNode* node) {
    AARNodeCmp* data = &node->data.cmp_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* left = eval(data->left);
    char* right = eval(data->right);

    sprintf(strbuf, "cmp %s, %s", right, left);

    free(right);
    free(left);

    return strbuf;
}

char* eval_jmp_stat(AARNode* node) {
    AARNodeJmp* data = &node->data.jmp_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* target = eval(data->target);
    sprintf(strbuf, "jmp %s", target);

    free(target);

    return strbuf;
}

char* eval_jeq_stat(AARNode* node) {
    AARNodeJmp* data = &node->data.jmp_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* target = eval(data->target);
    sprintf(strbuf, "jeq %s", target);

    free(target);

    return strbuf;
}

char* eval_jne_stat(AARNode* node) {
    AARNodeJmp* data = &node->data.jmp_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* target = eval(data->target);
    sprintf(strbuf, "jne %s", target);

    free(target);

    return strbuf;
}

char* eval_jlt_stat(AARNode* node) {
    AARNodeJmp* data = &node->data.jmp_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* target = eval(data->target);
    sprintf(strbuf, "jlt %s", target);

    free(target);

    return strbuf;
}

char* eval_jle_stat(AARNode* node) {
    AARNodeJmp* data = &node->data.jmp_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* target = eval(data->target);
    sprintf(strbuf, "jle %s", target);

    free(target);

    return strbuf;
}

char* eval_jgt_stat(AARNode* node) {
    AARNodeJmp* data = &node->data.jmp_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* target = eval(data->target);
    sprintf(strbuf, "jgt %s", target);

    free(target);

    return strbuf;
}

char* eval_jge_stat(AARNode* node) {
    AARNodeJmp* data = &node->data.jmp_stat;

    char* strbuf = malloc(256 * sizeof(char));

    char* target = eval(data->target);
    sprintf(strbuf, "jge %s", target);

    free(target);

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

char* eval_ident_lit(AARNode* node) {
    AARNodeIdent* data = &node->data.ident_lit;

    // TODO: Handle ident size
    char* strbuf = calloc(16, sizeof(char));
    // printf("%s %ld\n", data->ident, data->size);
    // memcpy(strbuf, data->ident, data->size);
    strncpy(strbuf, data->ident, 16);

    return strbuf;
}

char* eval(AARNode* node) {
    switch (node->type) {
        case AAR_NT_LABEL_STAT: return eval_label_stat(node);
        case AAR_NT_MOV_STAT: return eval_mov_stat(node);
        case AAR_NT_ADD_STAT: return eval_add_stat(node);
        case AAR_NT_SUB_STAT: return eval_sub_stat(node);
        case AAR_NT_MUL_STAT: return eval_mul_stat(node);
        case AAR_NT_DIV_STAT: return eval_div_stat(node);
        case AAR_NT_CMP_STAT: return eval_cmp_stat(node);
        case AAR_NT_JMP_STAT: return eval_jmp_stat(node);
        case AAR_NT_JEQ_STAT: return eval_jeq_stat(node);
        case AAR_NT_JNE_STAT: return eval_jne_stat(node);
        case AAR_NT_JLT_STAT: return eval_jlt_stat(node);
        case AAR_NT_JLE_STAT: return eval_jle_stat(node);
        case AAR_NT_JGT_STAT: return eval_jgt_stat(node);
        case AAR_NT_JGE_STAT: return eval_jge_stat(node);
        case AAR_NT_INT_LIT:   return eval_int_lit(node);
        case AAR_NT_IDENT_LIT: return eval_ident_lit(node);
        case AAR_NT_REG_LIT:   return eval_reg_lit(node);
        default: printf("%d\n", node->type); assert(false); break;
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
