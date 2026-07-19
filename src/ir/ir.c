#include "ir/ir.h"
#include "lexer/token.h"
#include "parser/parser.h"

#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char*  vn[256];
static LLVMValueRef vv[256];


LLVMValueRef build_node(IRBuilder* b, const Node* node);

LLVMValueRef build_int_lit(IRBuilder* b, const Node* node) {
    int32_t value = atoi(node->data.int_lit.value);

    LLVMValueRef result = LLVMConstInt(
        LLVMInt32TypeInContext(b->llvm_context),
        value,
        0
    );

    return result;
}

LLVMValueRef build_ident_lit(IRBuilder* b, const Node* node) {
    const NIdentLit* data = &node->data.ident_lit;

    for (int32_t i = 0; i < 256; ++i) {
        if (vn[i] == NULL)
            continue;

        if (strcmp(vn[i], data->value) == 0) {
            LLVMValueRef var_ptr = vv[i];
            LLVMValueRef var_val = LLVMBuildLoad2(b->llvm_builder, LLVMInt32TypeInContext(b->llvm_context), var_ptr, "valtmp");

            return var_val;
        }
    }

    assert(false);

    return NULL;
}

LLVMValueRef build_bin_expr(IRBuilder* b, const Node* node) {
    const NBinExpr* data = &node->data.bin_expr;

    LLVMValueRef left = build_node(b, data->left);
    LLVMValueRef right = build_node(b, data->right);

    assert(left != NULL);
    assert(right != NULL);

    switch (data->op) {
        case TT_PLUS:     return LLVMBuildAdd(b->llvm_builder, left, right, "addtmp"); break;
        case TT_MINUS:    return LLVMBuildSub(b->llvm_builder, left, right, "subtmp"); break;
        case TT_ASTERISK: return LLVMBuildMul(b->llvm_builder, left, right, "multmp"); break;
        case TT_SLASH:    return LLVMBuildSDiv(b->llvm_builder, left, right, "divtmp"); break;
        default: assert(false); break;
    }
}

LLVMValueRef build_call_expr(IRBuilder* b, const Node* node) {
    const NCallExpr* data = &node->data.call_expr;

    LLVMTypeRef  calleeT = LLVMGetTypeByName(b->llvm_module, data->member->data.ident_lit.value);
    LLVMValueRef calleeV = LLVMGetNamedFunction(b->llvm_module, data->member->data.ident_lit.value);

    assert(calleeT != NULL);
    assert(calleeV != NULL);

    if (data->args.count != LLVMCountParamTypes(calleeT)) {
        assert(false);
    }

    LLVMValueRef args[data->args.count];

    for (int32_t i = 0; i < data->args.count; ++i) {
        args[i] = build_node(b, &data->args.nodes[i]);
    }

    return LLVMBuildCall2(b->llvm_builder, calleeT, calleeV, args, data->args.count, "calltmp");
}

LLVMValueRef build_ret_stat(IRBuilder* b, const Node* node) {
    const NRetStat* data = &node->data.ret_stat;

    LLVMValueRef return_value = build_node(b, data->expr);
    return LLVMBuildRet(b->llvm_builder, return_value);
}

LLVMValueRef build_var_stat(IRBuilder* b, const Node* node) {
    const NVarStat* data = &node->data.var_stat;

    const char* var_ident = data->ident->data.ident_lit.value;
    LLVMValueRef var_ptr = LLVMBuildAlloca(b->llvm_builder, LLVMInt32TypeInContext(b->llvm_context), var_ident);

    for (int32_t i = 0; i < 256; ++i) {
        if (vn[i] == NULL) {
            vn[i] = var_ident;
            vv[i] = var_ptr;
            // printf("%s\n", vn[i]);
            break;
        }
    }

    if (data->value != NULL) {
        LLVMBuildStore(b->llvm_builder, build_node(b, data->value), var_ptr);
    }

    return NULL;
}

LLVMValueRef build_fn_stat(IRBuilder* b, const Node* node) {
    const NFuncStat* data = &node->data.func_stat;

    const char* func_ident    = data->ident->data.ident_lit.value;
    LLVMTypeRef func_params[] = { LLVMVoidTypeInContext(b->llvm_context) };
    LLVMTypeRef func_type     = LLVMFunctionType(
        LLVMInt32TypeInContext(b->llvm_context), func_params, 0, 0);

    if (LLVMGetTypeByName(b->llvm_module, func_ident) != NULL) {
        assert(false);
    }

    LLVMValueRef      func_value = LLVMAddFunction(b->llvm_module, func_ident, func_type);
    LLVMBasicBlockRef func_block = LLVMAppendBasicBlockInContext(b->llvm_context, func_value, func_ident);
    LLVMPositionBuilderAtEnd(b->llvm_builder, func_block);

    const NBlock* body_data = &data->body->data.block;
    for (int32_t i = 0; i < body_data->nodes.count; ++i) {
        build_node(b, &body_data->nodes.nodes[i]);
    }

    return NULL;
}

LLVMValueRef build_node(IRBuilder* b, const Node* node) {
    switch (node->type) {
        case NT_INTEGER_LIT: return build_int_lit(b, node);
        case NT_IDENT_LIT:   return build_ident_lit(b, node);
        case NT_BIN_EXPR:    return build_bin_expr(b, node);
        case NT_CALL_EXPR:   return build_call_expr(b, node);
        case NT_RETURN_STAT: return build_ret_stat(b, node);
        case NT_VAR_STAT:    return build_var_stat(b, node);
        case NT_FUNC_STAT:   return build_fn_stat(b, node);
        default: assert(false); break;
    }
}

IRBuilder create_ir_builder(const Node* ast, const DefTable* def_table) {
    IRBuilder b = (IRBuilder) { 0 };

    b.ast       = ast;
    b.def_table = def_table;

    b.llvm_context = LLVMContextCreate();
    b.llvm_module  = LLVMModuleCreateWithNameInContext("main", b.llvm_context);
    b.llvm_builder = LLVMCreateBuilderInContext(b.llvm_context);
    // b.llvm_entry   = LLVMCreateBasicBlockInContext(b.llvm_context, "entry");
    // LLVMPositionBuilderAtEnd(b.llvm_builder, b.llvm_entry);

    memset(vn, 0, sizeof(vn));
    memset(vv, 0, sizeof(vv));

    return b;
}

void free_ir_builder(IRBuilder* b) {
    LLVMDisposeBuilder(b->llvm_builder);
    LLVMDisposeModule(b->llvm_module);
    LLVMContextDispose(b->llvm_context);
}

void ir_builder_build(IRBuilder* b) {
    const NProgram* ast_data = &b->ast->data.program;

    for (int32_t i = 0; i < ast_data->nodes.count; ++i) {
        build_node(b, &ast_data->nodes.nodes[i]);
    }
}

void ir_builder_save_into_file(IRBuilder* b, const char* file) {
    int32_t success = LLVMWriteBitcodeToFile(b->llvm_module, file);

    if (success != 0) {
        printf("Failed to write LLVM bitcode into a file\n");
    }
}
