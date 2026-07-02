#include <interp.h>
#include <value.h>
#include <ast.h>
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
bool show_bytecode = false;
bool no_semantics = false;

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
    printf("        --no-semantics - Disable semantics (type checking)\n");
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
    show_bytecode = args_contains(argc, argv, "--p-bytecode");
    no_semantics = args_contains(argc, argv, "--no-semantics");

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

    // printf("The NodePool's used size is %ld bytes\n", parser.node_pool.count);

    /* Semantics Stage */
    if (!no_semantics) {
        if (show_stages)
            printf("Processing semantics...\n");

        Scope scope = create_scope(NULL);
        Semantics semantics = create_semantics(&scope);
        process_semantics(&semantics, &result);

        if (semantics.error) {
            printf("%s\n", get_semantics_error(&semantics));

            free_token_array(&token_array);
            free_lexer(&lexer);

            free(lexer_code);
            lexer_code = NULL;
            return;
        }
    }

    // if (show_stages)
    //     printf("Generating bytecode...\n");

    // Bytegen bytegen = {0};
    // init_bytegen(&bytegen, result);

    // bytegen_generate(&bytegen);

    // if (show_bytecode) {
    //     printf("\n==== BYTECODE ====\n");
    //     print_bytecode(&bytegen.result);
    //     printf("\n");
    // }

    // FILE* out_file = fopen("out.prb", "w");

    // fwrite(bytegen.result.bytes, sizeof(uint8_t), bytegen.result.count, out_file);

    // fclose(out_file);

    // free_bytecode(&bytegen.result);
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

    Interpreter interp = create_interpreter(ast);

    Semantics semantics = create_semantics(&interp.scope);

    /* Assigning core types */
    type_table_assign_type( &interp.scope.type_table, "i8",   create_value_typedef(VT_INT8)    );
    type_table_assign_type( &interp.scope.type_table, "i16",  create_value_typedef(VT_INT16)   );
    type_table_assign_type( &interp.scope.type_table, "i32",  create_value_typedef(VT_INT32)   );
    type_table_assign_type( &interp.scope.type_table, "i64",  create_value_typedef(VT_INT64)   );
    type_table_assign_type( &interp.scope.type_table, "u8",   create_value_typedef(VT_UINT8)   );
    type_table_assign_type( &interp.scope.type_table, "u16",  create_value_typedef(VT_UINT16)  );
    type_table_assign_type( &interp.scope.type_table, "u32",  create_value_typedef(VT_UINT32)  );
    type_table_assign_type( &interp.scope.type_table, "u64",  create_value_typedef(VT_UINT64)  );
    type_table_assign_type( &interp.scope.type_table, "f32",  create_value_typedef(VT_FLOAT32) );
    type_table_assign_type( &interp.scope.type_table, "f64",  create_value_typedef(VT_FLOAT64) );
    type_table_assign_type( &interp.scope.type_table, "bool", create_alias_typedef("u8")       );

    // Preloading
    scope_declare_var(&interp.scope, "null", create_value_typedef(VT_UINT8));
    scope_define_var(&interp.scope, "null", (Value) { .type = VT_UINT8, .value.u8 = 0 });

    scope_declare_var(&interp.scope, "false", create_value_typedef(VT_UINT8));
    scope_define_var(&interp.scope, "false", (Value) { .type = VT_UINT8, .value.u8 = 0 });

    scope_declare_var(&interp.scope, "true", create_value_typedef(VT_UINT8));
    scope_define_var(&interp.scope, "true", (Value) { .type = VT_UINT8, .value.u8 = 1 });

    process_semantics(&semantics, &ast);

    if (semantics.error) {
        printf("%s\n", get_semantics_error(&semantics));

        free_parser(&parser);
        free_token_array(&token_array);
        free_lexer(&lexer);

        free(lexer_code);
        lexer_code = NULL;
        return;
    }

    if (show_semantics) {
        printf("\n===== SEMANTICS ====\n");
        print_scope_structs(semantics.scope);
    }

    run_interpreter(&interp);

    free_interpreter(&interp);
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
