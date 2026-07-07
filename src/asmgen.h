#ifndef ASM_GEN_H
#define ASM_GEN_H

/***
 * Assembly Generation processes the AAR (Abstract Assembly Representation)
 * and generates the text of assembly code from it.
 * Has different versions.
 */

#include "aar.h"

typedef struct AsmGen {
    AARNode* aar;

    char*  result;
    size_t result_count;
} AsmGen;

AsmGen create_asm_gen(AARNode* aar);

void free_asm_gen(AsmGen* asm_gen);

/***
 * Pushes a string buffer onto the result buffer.
 */
void asm_gen_push(AsmGen* asm_gen, const char* sub);

/***
 * Declarations for different targets.
 */
void asm_gen_nasm(AsmGen* asm_gen);

#endif
