#include <bytecode.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <vm.h>
#include <opcode.h>

const OpCodeFunc OpCodeFuncs[] = {
    [OP_RET]  = NULL,
    [OP_CALL] = NULL,
    [OP_SYS]  = NULL,
    [OP_PSH]  = opcode_psh_func,
    [OP_POP]  = opcode_pop_func,
    [OP_LOAD] = opcode_load_func,
    [OP_STOR] = opcode_stor_func,
    [OP_NG]   = NULL,
    [OP_ADD]  = opcode_add_func,
    [OP_SUB]  = opcode_sub_func,
    [OP_MUL]  = opcode_mul_func,
    [OP_DIV]  = opcode_div_func,
    [OP_UMUL] = opcode_umul_func,
    [OP_UDIV] = opcode_udiv_func,
    [OP_FADD] = opcode_fadd_func,
    [OP_FSUB] = opcode_fsub_func,
    [OP_FMUL] = opcode_fmul_func,
    [OP_FDIV] = opcode_fdiv_func,
    [OP_INC]  = opcode_inc_func,
    [OP_DEC]  = opcode_dec_func,
    [OP_AND]  = NULL,
    [OP_OR]   = NULL,
    [OP_XOR]  = NULL,
    [OP_NOT]  = NULL,
    [OP_CMP]  = NULL,
    [OP_JMP]  = NULL,
    [OP_JEQ]  = NULL,
    [OP_JNE]  = NULL,
    [OP_JLT]  = NULL,
    [OP_JGT]  = NULL,
    [OP_JLE]  = NULL,
    [OP_JGE]  = NULL,
};

void init_vm(VM* vm, Bytecode bytecode) {
    vm->code = bytecode;
    vm->code_pointer = 0;

    memset(vm->stack, 0, sizeof(vm->stack));
    vm->stack_pointer = 0;

    vm->running = false;
    vm->errcode = VM_ERR_SUCCESS;
}

void start_vm(VM* vm) {
    vm->running = true;
    vm->errcode = VM_ERR_SUCCESS;

    while (vm->code_pointer < vm->code.count) {
        OpCode opcode = vm->code.bytes[vm->code_pointer];
        // printf("%s\n", OpCodeNames[opcode]);

        if (OpCodeFuncs[opcode] == NULL) {
            printf("OpCode function for %s not defined\n", OpCodeNames[opcode]);
            break;
        }

        OpCodeFuncs[opcode](vm);

        if (vm->errcode != VM_ERR_SUCCESS) {
            break;
        }
    }

    vm->running = false;
}

void push_vm(VM* vm, const uint8_t* value, size_t size) {
    memcpy(&vm->stack[vm->stack_pointer], value, size);
    vm->stack_pointer += size;
}

const uint8_t* pop_vm(VM* vm, size_t size) {
    vm->stack_pointer -= size;

    return &vm->stack[vm->stack_pointer];
}
