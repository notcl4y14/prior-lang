#include "../src/lexer.h"
#include "../src/parser.h"

#include <assert.h>
#include <string.h>

const char* code = "fn main(): void {} foo();";

int main(void) {
    Lexer lexer = create_lexer();
    load_lexer_code(&lexer, code, (size_t) strlen(code));

    TokenArray tokens = lexer_tokenize(&lexer);

    Parser parser = create_parser(tokens);
    Node ast = parse_tokens(&parser);

    assert(ast.type == NT_PROGRAM);
    assert(ast.data.program.nodes.count == 2);

    free_parser(&parser);
    free_token_array(&tokens);
    free_lexer(&lexer);
}
