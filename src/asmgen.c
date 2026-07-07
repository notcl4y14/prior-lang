#include "asmgen.h"
#include <stdlib.h>
#include <string.h>

AsmGen create_asm_gen(AARNode* aar) {
    AsmGen asm_gen = (AsmGen) { 0 };
    asm_gen.aar = aar;
    asm_gen.result = calloc(2048, sizeof(char));
    asm_gen.result_count = 0;

    return asm_gen;
}

void free_asm_gen(AsmGen* asm_gen) {
    free(asm_gen->result);
    asm_gen->result = NULL;
}

void asm_gen_push(AsmGen* asm_gen, const char* sub) {
    void*       copy_dst = &asm_gen->result[asm_gen->result_count];
    const void* copy_src = sub;
    size_t      copy_size = strlen(sub);

    memcpy(copy_dst, copy_src, copy_size);

    asm_gen->result_count += copy_size;
}
