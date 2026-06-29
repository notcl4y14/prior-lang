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

    // Check for correct node count
    assert(ast.data.program.nodes.count == 2);

    // Check for correct `fn main (): void {}` position range
    assert(ast.data.program.nodes.nodes[0].left_pos.column == 0);
    assert(ast.data.program.nodes.nodes[0].right_pos.column == 18);

    // Check for correct `foo()` position range
    assert(ast.data.program.nodes.nodes[1].left_pos.column == 19);
    assert(ast.data.program.nodes.nodes[1].right_pos.column == 24);

    free_parser(&parser);
    free_token_array(&tokens);
    free_lexer(&lexer);
}
