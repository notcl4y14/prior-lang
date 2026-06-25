#ifndef OPCODE_H
#define OPCODE_H

#include <vm.h>

void opcode_psh_func(VM* vm);
void opcode_pop_func(VM* vm);
void opcode_add_func(VM* vm);
void opcode_sub_func(VM* vm);
void opcode_mul_func(VM* vm);
void opcode_div_func(VM* vm);
void opcode_umul_func(VM* vm);
void opcode_udiv_func(VM* vm);
void opcode_fadd_func(VM* vm);
void opcode_fsub_func(VM* vm);
void opcode_fmul_func(VM* vm);
void opcode_fdiv_func(VM* vm);

#endif
