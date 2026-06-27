#include "interp.h"
#include "value.h"
#include <stdbool.h>
#include <stddef.h>
#include <vm.h>
#include <bytecode.h>
#include <ast.h>
#include <parser.h>
#include <lexer.h>
#include <semantics.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool show_stages = false;
bool show_tokens = false;
bool show_ast = false;
bool show_bytecode = false;
bool no_semantics = false;

bool args_contains(int32_t argc, char* argv[], const char* arg) {
    for (int32_t i = 0; i < argc; ++i) {
        if (strcmp(argv[i], arg) == 0) {
            return true;
        }
    }

    return false;
}

void read_file(const char* filename, char** output, size_t* filesize) {
    /* Open file */
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Failed to load file \"%s\"\n", filename);
        return;
    }

    /* Get file size */
    fseek(file, 0, SEEK_END);
    *filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    /* Allocating output */
    *output = malloc(*filesize + 1);

    /* Reading into output */
    fread(*output, sizeof(uint8_t), *filesize, file);

    /* Adding a null terminator */
    // *output[*filesize] = '\0';

    /* Close file */
    fclose(file);
}

void usage() {
    printf("Prior Compiler\n\n");
    // printf("No source files specified.\n");
    printf("Commands:\n");
    printf("    compile <file> - Compiles a file into bytecode out.prb\n");
    printf("        --p-stages - Print the stages of the compiling process\n");
    printf("        --p-tokens - Print the tokens of the code\n");
    printf("        --p-ast    - Print the AST of the code\n");
    printf("        --p-bytecode - Print the bytecode result\n");
    printf("        --no-semantics - Disable semantics (type checking)\n");
    printf("    run <file> - Start a VM and runs the bytecode file\n");
    printf("    interpret <file> - Interpret and run the file\n");
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

    Parser parser = create_parser(&token_array);
    Node result = parse_tokens(&parser);

    if (parser.error) {
        printf("%ld:%ld: %s\n", parser.errpos.line + 1, parser.errpos.column + 1, parser_get_error(&parser));

        free_node_pool(&parser.node_pool);
        free_token_array(&token_array);
        free_lexer(&lexer);

        free(lexer_code);
        lexer_code = NULL;
        return;
    }

    if (show_ast) {
        printf("\n==== AST ====\n");
        print_node_tree(result, 0);
    }

    // printf("The NodePool's used size is %ld bytes\n", parser.node_pool.count);

    /* Semantics Stage */
    if (!no_semantics) {
        if (show_stages)
            printf("Processing semantics...\n");

        Semantics semantics = create_semantics();
        process_semantics(&semantics, result);

        if (semantics.error) {
            printf("%s\n", get_semantics_error(&semantics));

            free_node_pool(&parser.node_pool);
            free_token_array(&token_array);
            free_lexer(&lexer);

            free(lexer_code);
            lexer_code = NULL;
            return;
        }
    }

    if (show_stages)
        printf("Generating bytecode...\n");

    Bytegen bytegen = {0};
    init_bytegen(&bytegen, result);

    bytegen_generate(&bytegen);

    if (show_bytecode) {
        printf("\n==== BYTECODE ====\n");
        print_bytecode(&bytegen.result);
        printf("\n");
    }

    FILE* out_file = fopen("out.prb", "w");

    fwrite(bytegen.result.bytes, sizeof(uint8_t), bytegen.result.count, out_file);

    fclose(out_file);

    free_bytecode(&bytegen.result);
    free_node_pool(&parser.node_pool);
    free_token_array(&token_array);
    free_lexer(&lexer);

    free(lexer_code);
    lexer_code = NULL;
}

void run(int32_t argc, char* argv[]) {
    Bytecode bytecode;

    char* filename = argv[2];
    FILE* input_file = fopen(filename, "r");

    fseek(input_file, 0, SEEK_END);
    size_t filesize = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    init_bytecode(&bytecode, filesize);
    fread(bytecode.bytes, sizeof(uint8_t), filesize, input_file);

    fclose(input_file);

    bytecode.count = filesize;
    bytecode.capacity = filesize;

    VM vm;
    init_vm(&vm, bytecode);

    start_vm(&vm);

    if (vm.errcode == VM_ERR_ZERO_DIVISION) {
        printf("Runtime error: Division by zero\n");
    }

    printf("Last stack value: [%d, %d, %d, %d]\n",
        *(int32_t*) &vm.stack[0],
        *(int32_t*) &vm.stack[4],
        *(int32_t*) &vm.stack[8],
        *(int32_t*) &vm.stack[12]
    );

    printf("Last int stack value: %d\n", *(int32_t*) &vm.stack[0]);
    printf("Last float stack value: %f\n", *(float*) &vm.stack[0]);

    free_bytecode(&bytecode);
}

void interpret(int32_t argc, char* argv[]) {
    /* Lexer Stage */
    char* filename = argv[2];
    char* lexer_code = NULL;
    size_t file_size = 0;

    read_file(filename, &lexer_code, &file_size);
    lexer_code[file_size] = 0;



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



    /* Parser Stage */
    Parser parser = create_parser(&token_array);
    Node result = parse_tokens(&parser);

    if (parser.error) {
        printf("%ld:%ld: %s\n", parser.errpos.line + 1, parser.errpos.column + 1, parser_get_error(&parser));

        free_node_pool(&parser.node_pool);
        free_token_array(&token_array);
        free_lexer(&lexer);

        free(lexer_code);
        lexer_code = NULL;
        return;
    }

    /* Semantics Stage */
    Semantics semantics = create_semantics();
    process_semantics(&semantics, result);

    if (semantics.error) {
        printf("%s\n", get_semantics_error(&semantics));

        free_node_pool(&parser.node_pool);
        free_token_array(&token_array);
        free_lexer(&lexer);

        free(lexer_code);
        lexer_code = NULL;
        return;
    }

    Interpreter interp = create_interpreter(result);
    // Preloading
    scope_declare_var(&interp.scope, "false");
    scope_declare_var(&interp.scope, "true");
    scope_define_var(&interp.scope, "false", (Value) { .type = VT_UINT8, .value.u8 = 0 });
    scope_define_var(&interp.scope, "true", (Value) { .type = VT_UINT8, .value.u8 = 1 });

    run_interpreter(&interp);

    free_interpreter(&interp);
    free_node_pool(&parser.node_pool);
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
    } else if (strcmp(argv[1], "run") == 0) {
        run(argc, argv);
    } else if (strcmp(argv[1], "interpret") == 0) {
        interpret(argc, argv);
    } else {
        printf("Unknown command %s\n", argv[1]);
    }

    return 0;
}
