#ifndef VM_H
#define VM_H

#include <bytecode.h>
#include <stddef.h>
#include <stdint.h>

#define STACK_SIZE (1024) // 1 KB

#define VM_ERR_SUCCESS 0x00
#define VM_ERR_ZERO_DIVISION 0x01

typedef struct VM {
    Bytecode code;
    uint32_t code_pointer;

    uint8_t  stack[STACK_SIZE];
    uint32_t stack_pointer;

    bool    running;
    uint8_t errcode;
} VM;

typedef void (*OpCodeFunc)(VM*);
extern const OpCodeFunc OpCodeFuncs[];

void init_vm(VM* vm, Bytecode bytecode);
void start_vm(VM* vm);
void push_vm(VM* vm, const uint8_t* value, size_t size);
const uint8_t* pop_vm(VM* vm, size_t size);

#endif
