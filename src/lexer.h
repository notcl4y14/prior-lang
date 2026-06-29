#ifndef LEXER_H
#define LEXER_H

#include <token.h>
#include <error.h>
#include <stddef.h>
#include <stdint.h>

#define LEXER_ERROR_SIZE 256

extern const char* TokenTypeNames[];

typedef struct TokenArray {
    Token* tokens;
    size_t count;
    size_t capacity;
} TokenArray;

void init_token_array(TokenArray* tarr, size_t capacity);
void free_token_array(TokenArray* tarr);
void push_token_array(TokenArray* tarr, Token token);
void print_token_array(const TokenArray* token_array);

typedef struct Lexer {
    char* code;
    size_t code_size;
    size_t cursor;

    TokenPosition position;
    ErrorList error_list;
} Lexer;

Lexer create_lexer();
void free_lexer(Lexer* lexer);
void load_lexer_code(Lexer* lexer, const char* code, size_t size);
/* Adds error to the Lexer's error_list */
void add_lexer_error(Lexer* lexer, const char* errmsg);
TokenArray lexer_tokenize(Lexer* lexer);

#endif
