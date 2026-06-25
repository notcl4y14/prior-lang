#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
#include <stdint.h>

typedef enum TokenType {
    TT_EOF,

    TT_OPERATOR,       // TODO: unused
    TT_INTEGER,
    TT_FLOAT,
    TT_STRING,
    TT_IDENTIFIER,

    TT_DOT,            // .
    TT_COMMA,          // ,
    TT_COLON,          // :
    TT_SEMICOLON,      // ;

    TT_PLUS,           // +
    TT_MINUS,          // -
    TT_ASTERISK,       // *
    TT_SLASH,          // /
    TT_MODULO,         // %
    TT_EQUAL,          // =
    TT_LESS,           // <
    TT_GREATER,        // >
    TT_AMPERSAND,      // &
    TT_PIPE,           // |
    TT_NOT,            // !

    TT_INCREMENT,      // ++
    TT_DECREMENT,      // --
    TT_PLUS_EQUAL,     // +=
    TT_MINUS_EQUAL,    // -=
    TT_TIMES_EQUAL,    // *=
    TT_DIVIDE_EQUAL,   // /=
    TT_MODULO_EQUAL,   // %=
    TT_EQUALS,         // ==
    TT_NOT_EQUALS,     // !=
    TT_LESS_EQUALS,    // <=
    TT_GREATER_EQUALS, // >=
    TT_AND,            // &&
    TT_OR,             // ||

    TT_LPAREN,         // (
    TT_RPAREN,         // )
    TT_LBRACKET,       // [
    TT_RBRACKET,       // ]
    TT_LBRACE,         // {
    TT_RBRACE,         // }

    TT_IF,
    TT_ELSE,
    TT_WHILE,
    TT_SWITCH,
    TT_CASE,
    TT_DEFAULT,
    TT_FN,
    TT_ENUM,
    TT_STRUCT,
    TT_VAR,
    TT_CONST,
    TT_BREAK,
    TT_CONTINUE,
    TT_RETURN,
} TokenType;

extern const char* TokenTypeNames[];

typedef struct TokenPosition {
    size_t line;
    size_t column;
} TokenPosition;

typedef struct Token {
    TokenType     type;
    char*         value;
    TokenPosition left_pos;
    TokenPosition right_pos;
} Token;

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
} Lexer;

Lexer create_lexer();
void free_lexer(Lexer* lexer);
void load_lexer_code(Lexer* lexer, const char* code, size_t size);
TokenArray lexer_tokenize(Lexer* lexer);

#endif
