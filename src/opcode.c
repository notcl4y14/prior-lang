#include "vm.h"
#include <opcode.h>
#include <stdint.h>
#include <stdio.h>

void opcode_psh_func(VM* vm) {
    vm->code_pointer++;
    uint8_t push_size = vm->code.bytes[vm->code_pointer++];
    uint8_t* value = &vm->code.bytes[vm->code_pointer + 1];

    push_vm(vm, value, push_size);
    vm->code_pointer += push_size;

    // printf("%.2f\n", *(float*) value);

    // vm->stack[vm->stack_pointer++] = value[0];
    // vm->stack[vm->stack_pointer++] = value[1];
    // vm->stack[vm->stack_pointer++] = value[2];
    // vm->stack[vm->stack_pointer++] = value[3];

    // printf("%.2f\n", *(float*) &vm->stack[vm->stack_pointer - 4]);

    // push_vm(vm, *(uint32_t*) value);
}

void opcode_pop_func(VM* vm) {
    vm->code_pointer++;
    uint8_t pop_size = vm->code.bytes[vm->code_pointer++];

    pop_vm(vm, pop_size);

    // vm->stack_pointer -= 4;
    // vm->code_pointer += 5;
}

void opcode_add_func(VM* vm) {
    vm->code_pointer++;

    uint32_t value1 = *(uint32_t*) pop_vm(vm, sizeof(uint32_t));
    uint32_t value2 = *(uint32_t*) pop_vm(vm, sizeof(uint32_t));

    // printf("%d + %d = %d\n", value1, value2, value1 + value2);

    uint32_t result = value1 + value2;
    push_vm(vm, (uint8_t*) &result, sizeof(uint32_t));
}

void opcode_sub_func(VM* vm) {
    vm->code_pointer++;

    uint32_t value1 = *(uint32_t*) pop_vm(vm, sizeof(uint32_t));
    uint32_t value2 = *(uint32_t*) pop_vm(vm, sizeof(uint32_t));

    uint32_t result = value1 - value2;
    push_vm(vm, (uint8_t*) &result, sizeof(uint32_t));
}

void opcode_mul_func(VM* vm) {
    vm->code_pointer++;

    int32_t value1 = *(int32_t*) pop_vm(vm, sizeof(int32_t));
    int32_t value2 = *(int32_t*) pop_vm(vm, sizeof(int32_t));

    int32_t result = value1 * value2;
    push_vm(vm, (uint8_t*) &result, sizeof(int32_t));
}

void opcode_div_func(VM* vm) {
    vm->code_pointer++;

    int32_t value1 = *(int32_t*) pop_vm(vm, sizeof(int32_t));
    int32_t value2 = *(int32_t*) pop_vm(vm, sizeof(int32_t));

    if (value2 == 0) {
        vm->errcode = VM_ERR_ZERO_DIVISION;
        return;
    }

    int32_t result = value1 / value2;
    push_vm(vm, (uint8_t*) &result, sizeof(int32_t));
}

void opcode_umul_func(VM* vm) {
    vm->code_pointer++;

    uint32_t value1 = *(uint32_t*) pop_vm(vm, sizeof(uint32_t));
    uint32_t value2 = *(uint32_t*) pop_vm(vm, sizeof(uint32_t));

    uint32_t result = value1 * value2;
    push_vm(vm, (uint8_t*) &result, sizeof(uint32_t));
}

void opcode_udiv_func(VM* vm) {
    vm->code_pointer++;

    uint32_t value1 = *(uint32_t*) pop_vm(vm, sizeof(uint32_t));
    uint32_t value2 = *(uint32_t*) pop_vm(vm, sizeof(uint32_t));

    if (value2 == 0) {
        vm->errcode = VM_ERR_ZERO_DIVISION;
        return;
    }

    uint32_t result = value1 / value2;
    push_vm(vm, (uint8_t*) &result, sizeof(uint32_t));
}

void opcode_fadd_func(VM* vm) {
    vm->code_pointer++;

    float value1 = *(float*) pop_vm(vm, sizeof(float));
    float value2 = *(float*) pop_vm(vm, sizeof(float));

    printf("%.2f + %.2f = %.2f\n", value1, value2, value1 + value2);

    float result = value1 + value2;
    push_vm(vm, (uint8_t*) &result, sizeof(float));
}

void opcode_fsub_func(VM* vm) {
    vm->code_pointer++;

    float value1 = *(float*) pop_vm(vm, sizeof(float));
    float value2 = *(float*) pop_vm(vm, sizeof(float));

    float result = value1 - value2;
    push_vm(vm, (uint8_t*) &result, sizeof(float));
}

void opcode_fmul_func(VM* vm) {
    vm->code_pointer++;

    float value1 = *(float*) pop_vm(vm, sizeof(float));
    float value2 = *(float*) pop_vm(vm, sizeof(float));

    float result = value1 * value2;
    push_vm(vm, (uint8_t*) &result, sizeof(float));
}

void opcode_fdiv_func(VM* vm) {
    vm->code_pointer++;

    float value1 = *(float*) pop_vm(vm, sizeof(float));
    float value2 = *(float*) pop_vm(vm, sizeof(float));

    if (value2 == 0.0f) {
        vm->errcode = VM_ERR_ZERO_DIVISION;
        return;
    }

    float result = value1 / value2;
    push_vm(vm, (uint8_t*) &result, sizeof(float));
}
