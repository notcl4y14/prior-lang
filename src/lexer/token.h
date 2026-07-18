#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

typedef enum TokenType {
    TT_EOF,            // EOF

    TT_INTEGER,        // Integer
    TT_FLOAT,          // Float
    TT_STRING,         // String
    TT_IDENTIFIER,     // Identifier

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

    TT_IF,             // if
    TT_ELSE,           // else
    TT_WHILE,          // while
    TT_SWITCH,         // switch
    TT_CASE,           // case
    TT_DEFAULT,        // default
    TT_FN,             // fn
    TT_ENUM,           // enum
    TT_STRUCT,         // struct
    TT_VAR,            // var
    TT_CONST,          // const
    TT_DEFER,          // defer
    TT_BREAK,          // break
    TT_CONTINUE,       // continue
    TT_RETURN,         // return
} TokenType;

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

#endif
