#include "lexer.h"
#include "utils.h"
#include <string.h>
#include <assert.h>

// See CMakeLists.txt
#define TEST_FILE       ROOT_DIR "/test/lexer.pr"

int main(void) {
    char* code = NULL;
    size_t code_size = 0;
    read_file(TEST_FILE, &code, &code_size);

    Lexer lexer = create_lexer();
    load_lexer_code(&lexer, code, code_size);
    TokenArray tokens = lexer_tokenize(&lexer);
    print_token_array(&tokens);

    assert(tokens.count == 8);

    free_token_array(&tokens);
    free_lexer(&lexer);
    return 0;
}