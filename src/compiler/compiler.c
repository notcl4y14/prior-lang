#include "compiler/compiler.h"
#include "ir/ir.h"
#include "lexer/lexer.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "semantics/scope.h"
#include "semantics/semantics.h"
#include "util/file.h"
#include "error.h"

#include <assert.h>
#include <stddef.h>



void print_tokens(TokenArray* tokens) {
    printf("\n==== TOKENS ====\n");
    print_token_array(tokens);
}

void print_ast(Node* ast) {
    printf("\n==== AST ====\n");
    print_node_tree(ast, 0);
}



void compile(const CompileOptions* options) {
    assert(options != NULL);
    assert(options->file != NULL);



    // Reading file contents
    char*  file_contents = NULL;
    size_t file_size     = 0;

    read_file(options->file, &file_contents, &file_size);
    file_contents[file_size] = 0; // null terminator

    assert(file_contents != NULL);



    // Generating tokens
    Lexer lexer = create_lexer();
    load_lexer_code(&lexer, file_contents, file_size);

    TokenArray tokens = lexer_tokenize(&lexer);

    if (lexer.error_list.count > 0) {
        print_error_list(&lexer.error_list);
        goto free_lexer;
    }

    if (options->show_tokens) {
        print_tokens(&tokens);
    }



    // Generating AST
    Parser parser = create_parser(tokens);
    Node ast = parse_tokens(&parser);

    if (parser.error) {
        printf("%ld:%ld: %s\n", parser.errpos.line + 1, parser.errpos.column + 1, parser_get_error(&parser));
        goto free_parser;
    }

    if (options->show_ast) {
        print_ast(&ast);
    }



    // Type checking and generating a SymTable
    Scope scope = create_scope(NULL);

    def_table_assign_def( &scope.def_table, "i32", create_type_def("i32", TYPE_VALUE_INT32) );

    Semantics semantics = create_semantics(&scope);
    process_semantics(&semantics, &ast);

    if (semantics.error_list.count > 0) {
        print_error_list(&semantics.error_list);
        goto free_semantics;
    }



    // Generating IR
    IRBuilder ir_builder = create_ir_builder(&ast, &scope.def_table);
    ir_builder_build(&ir_builder);
    ir_builder_save_into_file(&ir_builder, "output.bc");



    // Freeing
    free_ir:
    free_ir_builder(&ir_builder);

    free_semantics:
    free_scope(&scope);

    free_parser:
    free_parser(&parser);
    free_token_array(&tokens);

    free_lexer:
    free_lexer(&lexer);
    free(file_contents);
}
