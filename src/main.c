#include <interp.h>
#include <value.h>
#include <ast.h>
#include "aar.h"
#include "asmgen.h"
#include "parser.h"
#include "error.h"
#include <lexer.h>
#include <semantics.h>
#include "scope.h"
#include "type.h"
#include "utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool show_stages = false;
bool show_tokens = false;
bool show_ast = false;
bool show_semantics = false;
bool show_asm = false;

void print_tokens(TokenArray* tokens) {
    printf("\n==== TOKENS ====\n");
    print_token_array(tokens);
}

void print_ast(Node* ast) {
    printf("\n==== AST ====\n");
    print_node_tree(ast, 0);
}

bool args_contains(int32_t argc, char* argv[], const char* arg) {
    for (int32_t i = 0; i < argc; ++i) {
        if (strcmp(argv[i], arg) == 0) {
            return true;
        }
    }

    return false;
}

void usage() {
    printf("Prior Compiler\n\n");
    // printf("No source files specified.\n");
    printf("Commands:\n");
    printf("    compile <file> - Compiles a file into bytecode out.prb\n");
    printf("        --p-stages - Print the stages of the compiling process\n");
    printf("        --p-tokens - Print the tokens of the code\n");
    printf("        --p-ast    - Print the AST of the code\n");
    printf("        --p-semantics - Print the Semantics result of the AST\n");
    printf("        --p-asm    - Print the Assembly code result\n");
    printf("    interpret <file> - Interpret and run the file\n");
    printf("        --p-stages - Print the stages of the compiling process\n");
    printf("        --p-tokens - Print the tokens of the code\n");
    printf("        --p-ast    - Print the AST of the code\n");
    printf("        --p-semantics - Print the Semantics result of the AST\n");
}

void compile(int32_t argc, char* argv[]) {
    show_stages = args_contains(argc, argv, "--p-stages");
    show_tokens = args_contains(argc, argv, "--p-tokens");
    show_ast = args_contains(argc, argv, "--p-ast");
    show_asm = args_contains(argc, argv, "--p-asm");

    /* Lexer Stage */
    char* filename = argv[2];
    char* lexer_code = NULL;
    size_t file_size = 0;

    read_file(filename, &lexer_code, &file_size);
    lexer_code[file_size] = 0;


    if (show_stages)
        printf("Lexing Tokens...\n");

    Lexer lexer = create_lexer();
    load_lexer_code(&lexer, lexer_code, file_size);

    TokenArray token_array = lexer_tokenize(&lexer);

    if (lexer.error_list.count > 0) {
        for (int32_t i = 0; i < lexer.error_list.count; ++i) {
            const Error* error = &lexer.error_list.errors[i];
            printf("%ld:%ld: %s\n", error->position.line + 1, error->position.column + 1, error->errmsg);
        }

        free_token_array(&token_array);
        free_lexer(&lexer);

        free(lexer_code);
        lexer_code = NULL;
        return;
    }


    if (show_tokens) {
        printf("\n==== TOKENS ====\n");
        print_token_array(&token_array);
    }


    /* Parser Stage */
    if (show_stages)
        printf("\nParsing AST...\n");

    Parser parser = create_parser(token_array);
    Node result = parse_tokens(&parser);

    if (parser.error) {
        printf("%ld:%ld: %s\n", parser.errpos.line + 1, parser.errpos.column + 1, parser_get_error(&parser));

        free_token_array(&token_array);
        free_lexer(&lexer);

        free(lexer_code);
        lexer_code = NULL;
        return;
    }

    if (show_ast) {
        printf("\n==== AST ====\n");
        print_node_tree(&result, 0);
    }

    /* Semantics Stage */
    if (show_stages)
        printf("Processing semantics...\n");

    Scope scope = create_scope(NULL);
    Semantics semantics = create_semantics(&scope);
    process_semantics(&semantics, &result);

    if (semantics.error_list.count > 0) {
        for (int32_t i = 0; i < semantics.error_list.count; ++i) {
            const Error* error = &semantics.error_list.errors[i];
            printf("%ld:%ld: %s\n", error->position.line + 1, error->position.column + 1, error->errmsg);
        }

        free_token_array(&token_array);
        free_lexer(&lexer);

        free(lexer_code);
        lexer_code = NULL;
        return;
    }

    /* AAR Stage */
    if (show_stages)
        printf("Parsing AAR...\n");

    AARParser aar_parser = create_aar_parser(&result);
    aar_parser_parse(&aar_parser);

    /* Assembly Generation Stage */
    if (show_stages)
        printf("Generating Assembly code...\n");

    AsmGen asm_gen = create_asm_gen(&aar_parser.result);
    asm_gen_nasm(&asm_gen);

    if (show_asm) {
        printf("\n==== Assembly ====\n");
        printf("%s", asm_gen.result);
    }

    free_asm_gen(&asm_gen);
    free_aar_parser(&aar_parser);
    free_parser(&parser);
    free_token_array(&token_array);
    free_lexer(&lexer);

    free(lexer_code);
    lexer_code = NULL;
}

void interpret(int32_t argc, char* argv[]) {
    show_stages = args_contains(argc, argv, "--p-stages");
    show_tokens = args_contains(argc, argv, "--p-tokens");
    show_ast = args_contains(argc, argv, "--p-ast");
    show_semantics = args_contains(argc, argv, "--p-semantics");

    /* Lexer Stage */
    char* filename = argv[2];
    char* lexer_code = NULL;
    size_t file_size = 0;

    read_file(filename, &lexer_code, &file_size);
    lexer_code[file_size] = 0;



    if (show_stages) printf("Lexing Tokens...\n");

    Lexer lexer = create_lexer();
    load_lexer_code(&lexer, lexer_code, file_size);

    TokenArray token_array = lexer_tokenize(&lexer);

    if (lexer.error_list.count > 0) {
        for (int32_t i = 0; i < lexer.error_list.count; ++i) {
            const Error* error = &lexer.error_list.errors[i];
            printf("%ld:%ld: %s\n", error->position.line + 1, error->position.column + 1, error->errmsg);
        }

        free_token_array(&token_array);
        free_lexer(&lexer);

        free(lexer_code);
        lexer_code = NULL;
        return;
    }

    if (show_tokens) print_tokens(&token_array);



    /* Parser Stage */
    if (show_stages) printf("\nParsing AST...\n");

    Parser parser = create_parser(token_array);
    Node ast = parse_tokens(&parser);

    if (parser.error) {
        printf("%ld:%ld: %s\n", parser.errpos.line + 1, parser.errpos.column + 1, parser_get_error(&parser));

        free_parser(&parser);
        free_token_array(&token_array);
        free_lexer(&lexer);

        free(lexer_code);
        lexer_code = NULL;
        return;
    }

    if (show_ast) print_ast(&ast);



    /* Semantics Stage */
    if (show_stages) printf("Processing semantics...\n");

    Scope scope = create_scope(NULL);

    Semantics semantics = create_semantics(&scope);

    /* Assigning core types */
    def_table_assign_def( &scope.def_table, "i8",    create_type_def("i8",   TYPE_VALUE_INT8)    );
    def_table_assign_def( &scope.def_table, "i16",   create_type_def("i16",  TYPE_VALUE_INT16)   );
    def_table_assign_def( &scope.def_table, "i32",   create_type_def("i32",  TYPE_VALUE_INT32)   );
    def_table_assign_def( &scope.def_table, "i64",   create_type_def("i64",  TYPE_VALUE_INT64)   );
    def_table_assign_def( &scope.def_table, "u8",    create_type_def("u8",   TYPE_VALUE_UINT8)   );
    def_table_assign_def( &scope.def_table, "u16",   create_type_def("u16",  TYPE_VALUE_UINT16)  );
    def_table_assign_def( &scope.def_table, "u32",   create_type_def("u32",  TYPE_VALUE_UINT32)  );
    def_table_assign_def( &scope.def_table, "u64",   create_type_def("u64",  TYPE_VALUE_UINT64)  );
    def_table_assign_def( &scope.def_table, "f32",   create_type_def("f32",  TYPE_VALUE_FLOAT32) );
    def_table_assign_def( &scope.def_table, "f64",   create_type_def("f64",  TYPE_VALUE_FLOAT64) );
    def_table_assign_def( &scope.def_table, "bool", create_alias_def("bool", "u8")               );

    // Preloading
    scope_declare_var(&scope, "null", create_type_def("null", TYPE_VALUE_UINT8).data.data_type);
    scope_define_var(&scope, "null", (Value) { .type = VT_UINT8, .value.u8 = 0 });

    scope_declare_var(&scope, "false", create_type_def("false", TYPE_VALUE_UINT8).data.data_type);
    scope_define_var(&scope, "false", (Value) { .type = VT_UINT8, .value.u8 = 0 });

    scope_declare_var(&scope, "true", create_type_def("true", TYPE_VALUE_UINT8).data.data_type);
    scope_define_var(&scope, "true", (Value) { .type = VT_UINT8, .value.u8 = 1 });

    process_semantics(&semantics, &ast);

    if (semantics.error_list.count > 0) {
        for (int32_t i = 0; i < semantics.error_list.count; ++i) {
            const Error* error = &semantics.error_list.errors[i];
            printf("%ld:%ld: %s\n", error->position.line + 1, error->position.column + 1, error->errmsg);
        }

        free_token_array(&token_array);
        free_lexer(&lexer);

        free(lexer_code);
        lexer_code = NULL;
        return;
    }

    if (show_semantics) {
        printf("\n===== SEMANTICS ====\n");
        print_scope_enums(semantics.scope);
        print_scope_structs(semantics.scope);
        print_scope_functions(semantics.scope);
    }

    Interpreter interp = create_interpreter(ast, &scope);

    run_interpreter(&interp);

    free_interpreter(&interp);
    free_scope(&scope);
    free_parser(&parser);
    free_token_array(&token_array);
    free_lexer(&lexer);

    free(lexer_code);
    lexer_code = NULL;
}

int32_t main(int32_t argc, char* argv[]) {
    /* subtracting the count by 1 because the first arg is executable */
    int32_t argument_count = argc - 1;

    if (argument_count == 0) {
        usage();
        return 0;
    }

    if (strcmp(argv[1], "compile") == 0) {
        compile(argc, argv);
    } else if (strcmp(argv[1], "interpret") == 0) {
        interpret(argc, argv);
    } else {
        printf("Unknown command %s\n", argv[1]);
    }

    return 0;
}
