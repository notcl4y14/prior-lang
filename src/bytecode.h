#ifndef BYTECODE_H
#define BYTECODE_H

#include <stddef.h>
#include <parser.h>
#include <stdint.h>

typedef enum OpCode {
    OP_RET,  // return
    OP_CALL, // call

    OP_SYS, // syscall

    OP_PSH, // push
    OP_POP, // pop

    OP_LOAD, // load
    OP_STOR, // store

    OP_NG,   // negate
    OP_ADD,  // add
    OP_SUB,  // subtract
    OP_MUL,  // multiply
    OP_DIV,  // divide
    OP_UMUL, // unsigned mul
    OP_UDIV, // unsigned div

    OP_FADD, // float add
    OP_FSUB, // float subtract
    OP_FMUL, // float multiply
    OP_FDIV, // float divide

    OP_AND, // and
    OP_OR,  // or
    OP_XOR, // xor
    OP_NOT, // not

    OP_CMP, // compare
    OP_JMP, // jump
    OP_JEQ, // jump if equal
    OP_JNE, // jump if not equal
    OP_JLT, // jump if less than
    OP_JGT, // jump if greater than
    OP_JLE, // jump if less than or equal
    OP_JGE, // jump if greater than or equal
} OpCode;

extern const char* OpCodeNames[];

typedef struct Bytecode {
    uint8_t* bytes;
    size_t   count;
    size_t   capacity;
} Bytecode;

void init_bytecode(Bytecode* bc, size_t size);
void free_bytecode(Bytecode* bc);
void print_bytecode(Bytecode* bc);

typedef struct Bytegen {
    Node     ast;
    Bytecode result;
} Bytegen;

void init_bytegen(Bytegen* bg, Node ast);
Bytecode bytegen_generate(Bytegen* bg);

#endif
