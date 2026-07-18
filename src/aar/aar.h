#ifndef AAR_H
#define AAR_H

#include "parser/parser.h"

#include <stddef.h>

/***
 * AAR (Abstract Assembly Representation) - an AST-like representation
 * of the assembly code used for constructing assembly for
 * multiple platforms and architectures by the AsmGen.
 */

typedef struct AARNode AARNode;

typedef enum AARRegister {
    // 32-bit
    AARREG_EAX,
    AARREG_EBX,
    AARREG_ECX,
    AARREG_EDX,
    // 64-bit
    AARREG_RAX,
    AARREG_RBX,
    AARREG_RCX,
    AARREG_RDX,
} AARRegister;

typedef enum AARNodeType {
    AAR_NT_NONE,

    AAR_NT_PROGRAM,

    // label
    AAR_NT_LABEL_STAT,
    // move
    AAR_NT_MOV_STAT,
    // mathematical operations
    AAR_NT_ADD_STAT,
    AAR_NT_SUB_STAT,
    AAR_NT_MUL_STAT,
    AAR_NT_DIV_STAT,
    // logical operations
    AAR_NT_AND_STAT,
    AAR_NT_OR_STAT,
    AAR_NT_NOT_STAT,
    AAR_NT_XOR_STAT,
    // comparative operations
    AAR_NT_CMP_STAT,
    AAR_NT_JMP_STAT,
    AAR_NT_JEQ_STAT,
    AAR_NT_JNE_STAT,
    AAR_NT_JLE_STAT,
    AAR_NT_JGE_STAT,
    AAR_NT_JLT_STAT,
    AAR_NT_JGT_STAT,

    AAR_NT_BIN_EXPR,
    AAR_NT_ADDR_EXPR,

    AAR_NT_INT_LIT,
    AAR_NT_IDENT_LIT,
    AAR_NT_REG_LIT,
} AARNodeType;

typedef struct AARNodeProgram {
    AARNode* nodes;
    size_t   count;
} AARNodeProgram;

typedef struct AARNodeLabel {
    char* name;
} AARNodeLabel;

typedef struct AARNodeMov {
    AARNode* src;
    AARNode* dst;
} AARNodeMov;

typedef struct AARNodeAdd {
    AARNode* src;
    AARNode* dst;
} AARNodeAdd;

typedef struct AARNodeSub {
    AARNode* src;
    AARNode* dst;
} AARNodeSub;

typedef struct AARNodeMul {
    AARNode* src;
    AARNode* dst;
} AARNodeMul;

typedef struct AARNodeDiv {
    AARNode* src;
    AARNode* dst;
} AARNodeDiv;

typedef struct AARNodeAnd {
    AARNode* left;
    AARNode* right;
} AARNodeAnd;

typedef struct AARNodeOr {
    AARNode* left;
    AARNode* right;
} AARNodeOr;

typedef struct AARNodeNot {
    AARNode* reg;
} AARNodeNot;

typedef struct AARNodeXor {
    AARNode* left;
    AARNode* right;
} AARNodeXor;

typedef struct AARNodeCmp {
    AARNode* left;
    AARNode* right;
} AARNodeCmp;

typedef struct AARNodeJmp {
    AARNode* target;
} AARNodeJmp;

typedef struct AARNodeBinExpr {
    AARNode* left;
    AARNode* right;
} AARNodeBinExpr;

typedef struct AARNodeInt {
    char*  value;
    size_t size;
} AARNodeInt;

typedef struct AARNodeIdent {
    char*  ident;
    size_t size;
} AARNodeIdent;

typedef struct AARNodeReg {
    AARRegister reg;
} AARNodeReg;

typedef struct AARNode {
    AARNodeType type;
    union {
        AARNodeProgram program;
        AARNodeLabel   label_stat;
        AARNodeMov     mov_stat;
        AARNodeAdd     add_stat;
        AARNodeSub     sub_stat;
        AARNodeMul     mul_stat;
        AARNodeDiv     div_stat;
        AARNodeAnd     and_stat;
        AARNodeOr      or_stat;
        AARNodeNot     not_stat;
        AARNodeXor     xor_stat;
        AARNodeCmp     cmp_stat;
        AARNodeJmp     jmp_stat;
        AARNodeInt     int_lit;
        AARNodeIdent   ident_lit;
        AARNodeReg     reg_lit;
    } data;
} AARNode;



typedef struct AARParser {
    Node*   ast;
    AARNode result;

    int32_t i32_reg;
    int32_t i64_reg;
    int32_t float_reg;
    int32_t label_count;
} AARParser;

AARParser create_aar_parser(Node* ast);
void free_aar_parser(AARParser* p);

void aar_parser_parse(AARParser* p);

#endif
