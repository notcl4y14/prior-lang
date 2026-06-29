#include "../src/lexer.h"
#include <string.h>
#include <assert.h>

const char* code = "main 123 () :";

int main(void) {
    Lexer lexer = create_lexer();
    load_lexer_code(&lexer, code, (size_t)strlen(code));
    TokenArray tokens = lexer_tokenize(&lexer);
    print_token_array(&tokens);

    assert(tokens.count == 6);
    // TODO: more tests

    free_token_array(&tokens);
    free_lexer(&lexer);
    return 0;
}