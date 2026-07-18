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

    AAR_NT_LABEL_STAT,
    AAR_NT_MOV_STAT,
    AAR_NT_ADD_STAT,
    AAR_NT_SUB_STAT,
    AAR_NT_MUL_STAT,
    AAR_NT_DIV_STAT,
    AAR_NT_AND_STAT,
    AAR_NT_OR_STAT,
    AAR_NT_NOT_STAT,
    AAR_NT_XOR_STAT,

    AAR_NT_BIN_EXPR,
    AAR_NT_ADDR_EXPR,

    AAR_NT_INT_LIT,
    AAR_NT_REG_LIT,
} AARNodeType;

typedef struct AARNProgram {
    AARNode* nodes;
    size_t   count;
} AARNProgram;

typedef struct AARNLabel {
    char* name;
} AARNLabel;

typedef struct AARNMov {
    AARNode* src;
    AARNode* dst;
} AARNMov;

typedef struct AARNAdd {
    AARNode* src;
    AARNode* dst;
} AARNAdd;

typedef struct AARNSub {
    AARNode* src;
    AARNode* dst;
} AARNSub;

typedef struct AARNMul {
    AARNode* src;
    AARNode* dst;
} AARNMul;

typedef struct AARNDiv {
    AARNode* src;
    AARNode* dst;
} AARNDiv;

typedef struct AARNAnd {
    AARNode* left;
    AARNode* right;
} AARNAnd;

typedef struct AARNOr {
    AARNode* left;
    AARNode* right;
} AARNOr;

typedef struct AARNNot {
    AARNode* reg;
} AARNNot;

typedef struct AARNXor {
    AARNode* left;
    AARNode* right;
} AARNXor;

typedef struct AARNBinExpr {
    AARNode* left;
    AARNode* right;
} AARNBinExpr;

typedef struct AARNReg {
    AARRegister reg;
} AARNReg;

typedef struct AARNInt {
    char*  value;
    size_t size;
} AARNInt;

typedef struct AARNode {
    AARNodeType type;
    union {
        AARNProgram program;
        AARNLabel label;
        AARNMov   mov;
        AARNAdd   add;
        AARNSub   sub;
        AARNMul   mul;
        AARNDiv   div;
        AARNAnd   and_;
        AARNOr    or_;
        AARNNot   not_;
        AARNXor   xor_;
        AARNReg   reg;
        AARNInt   int_;
    } data;
} AARNode;



typedef struct AARParser {
    Node*   ast;
    AARNode result;
} AARParser;

AARParser create_aar_parser(Node* ast);
void free_aar_parser(AARParser* p);

void aar_parser_parse(AARParser* p);

#endif
