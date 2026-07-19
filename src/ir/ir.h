#ifndef IR_H
#define IR_H

#include "parser/parser.h"
#include "type.h"

#include <llvm-c/Types.h>

typedef struct IRBuilder {
    const Node*     ast;
    const DefTable* def_table;

    LLVMContextRef llvm_context;
    LLVMModuleRef  llvm_module;
    LLVMBuilderRef llvm_builder;
    LLVMBasicBlockRef llvm_entry;
} IRBuilder;

IRBuilder create_ir_builder(const Node* ast, const DefTable* def_table);
void free_ir_builder(IRBuilder* b);
void ir_builder_build(IRBuilder* b);
void ir_builder_save_into_file(IRBuilder* b, const char* file);

#endif
