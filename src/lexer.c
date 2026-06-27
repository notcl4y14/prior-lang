#include <assert.h>
#include <mem.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <lexer.h>

const char* TokenTypeNames[] = {
    [TT_EOF] = "EOF",

    [TT_INTEGER]    = "INTEGER",
    [TT_FLOAT]      = "FLOAT",
    [TT_STRING]     = "STRING",
    [TT_IDENTIFIER] = "IDENTIFIER",

    [TT_DOT]       = "DOT",
    [TT_COMMA]     = "COMMA",
    [TT_COLON]     = "COLON",
    [TT_SEMICOLON] = "SEMICOLON",

    [TT_PLUS]      = "PLUS",
    [TT_MINUS]     = "MINUS",
    [TT_ASTERISK]  = "ASTERISK",
    [TT_SLASH]     = "SLASH",
    [TT_MODULO]    = "MODULO",
    [TT_EQUAL]     = "EQUAL",
    [TT_LESS]      = "LESS",
    [TT_GREATER]   = "GREATER",
    [TT_AMPERSAND] = "AMPERSAND",
    [TT_PIPE]      = "PIPE",
    [TT_NOT]       = "NOT",

    [TT_INCREMENT]      = "INCREMENT",
    [TT_DECREMENT]      = "DECREMENT",
    [TT_PLUS_EQUAL]     = "PLUS_EQUAL",
    [TT_MINUS_EQUAL]    = "MINUS_EQUAL",
    [TT_TIMES_EQUAL]    = "TIMES_EQUAL",
    [TT_DIVIDE_EQUAL]   = "DIVIDE_EQUAL",
    [TT_MODULO_EQUAL]   = "MODULO_EQUAL",
    [TT_EQUALS]         = "EQUALS",
    [TT_NOT_EQUALS]     = "NOT_EQUALS",
    [TT_LESS_EQUALS]    = "LESS_EQUALS",
    [TT_GREATER_EQUALS] = "GREATER_EQUALS",
    [TT_AND]            = "AND",
    [TT_OR]             = "OR",

    [TT_LPAREN]   = "LPAREN",
    [TT_RPAREN]   = "RPAREN",
    [TT_LBRACKET] = "LBRACKET",
    [TT_RBRACKET] = "RBRACKET",
    [TT_LBRACE]   = "LBRACE",
    [TT_RBRACE]   = "RBRACE",

    [TT_IF]       = "IF",
    [TT_ELSE]     = "ELSE",
    [TT_WHILE]    = "WHILE",
    [TT_SWITCH]   = "SWITCH",
    [TT_CASE]     = "CASE",
    [TT_DEFAULT]  = "DEFAULT",
    [TT_FN]       = "FN",
    [TT_ENUM]     = "ENUM",
    [TT_STRUCT]   = "STRUCT",
    [TT_VAR]      = "VAR",
    [TT_CONST]    = "CONST",
    [TT_BREAK]    = "BREAK",
    [TT_CONTINUE] = "CONTINUE",
    [TT_RETURN]   = "RETURN",
};

void init_token_array(TokenArray* tarr, size_t capacity) {
    tarr->tokens = (Token*) calloc(capacity, sizeof(Token));
    assert(tarr->tokens != NULL && "Failed to allocate token array");

    tarr->count = 0;
    tarr->capacity = capacity;
}

void free_token_array(TokenArray* tarr) {
    for (size_t i = 0; i < tarr->count; ++i) {
        Token* token = &tarr->tokens[i];

        if (token->value != NULL) {
            free(token->value);
        }
    }

    free(tarr->tokens);

    tarr->tokens = NULL;
}

void push_token_array(TokenArray* tarr, Token token) {
    if (tarr->count >= tarr->capacity) {
        tarr->capacity *= 2;
        tarr->tokens = (Token*) realloc(tarr->tokens, tarr->capacity * sizeof(Token));
        assert(tarr->tokens != NULL && "Failed to reallocate token array");
    }

    tarr->tokens[tarr->count++] = token;
}

void print_token_array(const TokenArray* token_array) {
    for (size_t i = 0; i < token_array->count; ++i) {
        const Token* token = &token_array->tokens[i];

        printf("[%s]", TokenTypeNames[token->type]);

        if (token->value != NULL) {
            printf(": \"%s\"", token->value);
        }

        printf(" (%ld:%ld - %ld:%ld)",
            token->left_pos.line + 1,
            token->left_pos.column + 1,
            token->right_pos.line + 1,
            token->right_pos.column + 1
        );

        printf("\n");
    }
}

bool is_char_whitespace(char c) {
    switch (c) {
        case ' ':
        case '\r':
        case '\n':
        case '\t':
            return true;
    }

    return false;
}

bool is_char_digit(char c) {
    // TODO: make a simple (x > y && x < z) check for digits if needed
    switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return true;
    }

    return false;
}

bool is_char_operator(char c) {
    switch (c) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '^':
        case '=':
        case '<':
        case '>':
        case '&':
        case '|':
        case '!':
            return true;
    }

    return false;
}

bool is_char_ident_symbol(char c) {
    if (c == '_') {
        return true;
    } else if (c >= 'a' && c <= 'z') {
        return true;
    } else if (c >= 'A' && c <= 'Z') {
        return true;
    }

    return false;
}

bool is_char_symbol(char c) {
    switch (c) {
        case '.':
        case ',':
        case ':':
        case ';':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
            return true;
    }

    return false;
}

char lexer_at(const Lexer* lexer, int32_t delta) {
    // TODO: fix this
    if ((int32_t) lexer->cursor + delta < 0) {
        printf("Tried to access lexer code out of bounds.\n");
        return '\0';
    }

    return lexer->code[lexer->cursor + delta];
}

char lexer_step(Lexer* lexer) {
    lexer->position.column += 1;
    if (lexer_at(lexer, -1) == '\n') {
        lexer->position.line += 1;
        lexer->position.column = 0;
    }

    char current_char = lexer->code[lexer->cursor++];
    return current_char;
}

char lexer_step_back(Lexer* lexer) {
    char current_char = lexer->code[--lexer->cursor];

    // NOTE: This doesn't work with lines
    lexer->position.column -= 1;

    return current_char;
}

Token lexer_tokenize_number(Lexer* lexer) {
    char strbuf[256] = {0};
    int32_t strbuf_c = 0;
    bool has_dot = false;

    TokenPosition left_pos = lexer->position;

    while (lexer->cursor < lexer->code_size) {
        char current_char = lexer_step(lexer);

        if (is_char_digit(current_char)) {
            strbuf[strbuf_c++] = current_char;
        } else if (current_char == '.') {
            if (has_dot) {
                break;
            }

            has_dot = true;
            strbuf[strbuf_c++] = current_char;
        } else {
            break;
        }
    }

    strbuf[strbuf_c++] = 0;

    lexer_step_back(lexer);
    TokenPosition right_pos = lexer->position;

    Token token = (Token) {0};
    token.type = has_dot ? TT_FLOAT : TT_INTEGER;
    token.value = str_alloc_copy(strbuf);
    assert(token.value != NULL && "Failed to allocate string token value for number");

    token.left_pos = left_pos;
    token.right_pos = right_pos;

    return token;
}

Token lexer_tokenize_operator(Lexer* lexer) {
    TokenPosition left_pos = lexer->position;

    char char1 = lexer_step(lexer);
    char char2 = lexer_at(lexer, 0);

    Token token = (Token) {0};

    // Two-char operators
    if (is_char_operator(char2)) {
        if (char1 == '+' && char2 == '+') token.type = TT_INCREMENT;
        else if (char1 == '-' && char2 == '-') token.type = TT_DECREMENT;
        else if (char1 == '+' && char2 == '=') token.type = TT_PLUS_EQUAL;
        else if (char1 == '-' && char2 == '=') token.type = TT_MINUS_EQUAL;
        else if (char1 == '*' && char2 == '=') token.type = TT_TIMES_EQUAL;
        else if (char1 == '/' && char2 == '=') token.type = TT_DIVIDE_EQUAL;
        else if (char1 == '%' && char2 == '=') token.type = TT_MODULO_EQUAL;
        else if (char1 == '=' && char2 == '=') token.type = TT_EQUALS;
        else if (char1 == '!' && char2 == '=') token.type = TT_NOT_EQUALS;
        else if (char1 == '<' && char2 == '=') token.type = TT_LESS_EQUALS;
        else if (char1 == '>' && char2 == '=') token.type = TT_GREATER_EQUALS;
        else if (char1 == '&' && char2 == '&') token.type = TT_AND;
        else if (char1 == '|' && char2 == '|') token.type = TT_OR;

        // Consuming the second char of the operator
        lexer_step(lexer);
    }
    // One-char operators
    else {
        if (char1 == '+') token.type = TT_PLUS;
        else if (char1 == '-') token.type = TT_MINUS;
        else if (char1 == '*') token.type = TT_ASTERISK;
        else if (char1 == '/') token.type = TT_SLASH;
        else if (char1 == '%') token.type = TT_MODULO;
        else if (char1 == '=') token.type = TT_EQUAL;
        else if (char1 == '<') token.type = TT_LESS;
        else if (char1 == '>') token.type = TT_GREATER;
        else if (char1 == '&') token.type = TT_AMPERSAND;
        else if (char1 == '|') token.type = TT_PIPE;
        else if (char1 == '!') token.type = TT_NOT;
    }

    TokenPosition right_pos = lexer->position;

    token.left_pos = left_pos;
    token.right_pos = right_pos;

    return token;
}

Token lexer_tokenize_string(Lexer* lexer) {
    char strbuf[256] = {0};
    int32_t strbuf_c = 0;

    TokenPosition left_pos = lexer->position;

    lexer_step(lexer);

    while (lexer->cursor < lexer->code_size) {
        char current_char = lexer_step(lexer);

        if (current_char == '"') {
            break;
        } else {
            strbuf[strbuf_c++] = current_char;
        }
    }

    strbuf[strbuf_c++] = 0;

    TokenPosition right_pos = lexer->position;

    Token token = (Token) {0};
    token.type = TT_STRING;
    token.value = str_alloc_copy(strbuf);
    assert(token.value != NULL && "Failed to allocate string token value for string");

    token.left_pos = left_pos;
    token.right_pos = right_pos;

    return token;
}

Token lexer_tokenize_ident(Lexer* lexer) {
    char strbuf[256] = {0};
    int32_t strbuf_c = 0;

    TokenPosition left_pos = lexer->position;

    while (lexer->cursor < lexer->code_size) {
        char current_char = lexer_at(lexer, 0);

        if (is_char_ident_symbol(current_char) || is_char_digit(current_char)) {
            strbuf[strbuf_c++] = current_char;
        } else {
            break;
        }

        lexer_step(lexer);
    }

    strbuf[strbuf_c++] = 0;

    Token token = (Token) {0};
    TokenPosition right_pos = lexer->position;

    token.left_pos = left_pos;
    token.right_pos = right_pos;

    /* Handling keywords */
    if (strcmp(strbuf, "if") == 0) {
        token.type = TT_IF;
        return token;
    } else if (strcmp(strbuf, "else") == 0) {
        token.type = TT_ELSE;
        return token;
    } else if (strcmp(strbuf, "while") == 0) {
        token.type = TT_WHILE;
        return token;
    } else if (strcmp(strbuf, "switch") == 0) {
        token.type = TT_SWITCH;
        return token;
    } else if (strcmp(strbuf, "case") == 0) {
        token.type = TT_CASE;
        return token;
    } else if (strcmp(strbuf, "default") == 0) {
        token.type = TT_DEFAULT;
        return token;
    } else if (strcmp(strbuf, "fn") == 0) {
        token.type = TT_FN;
        return token;
    } else if (strcmp(strbuf, "enum") == 0) {
        token.type = TT_ENUM;
        return token;
    } else if (strcmp(strbuf, "struct") == 0) {
        token.type = TT_STRUCT;
        return token;
    } else if (strcmp(strbuf, "var") == 0) {
        token.type = TT_VAR;
        return token;
    } else if (strcmp(strbuf, "const") == 0) {
        token.type = TT_CONST;
        return token;
    } else if (strcmp(strbuf, "break") == 0) {
        token.type = TT_BREAK;
        return token;
    } else if (strcmp(strbuf, "continue") == 0) {
        token.type = TT_CONTINUE;
        return token;
    } else if (strcmp(strbuf, "return") == 0) {
        token.type = TT_RETURN;
        return token;
    }

    token.type = TT_IDENTIFIER;
    token.value = str_alloc_copy(strbuf);
    assert(token.value != NULL && "Failed to allocate string token value for string");

    return token;
}

Token lexer_tokenize_symbol(Lexer* lexer) {
    Token token = (Token) {0};
    TokenPosition left_pos = lexer->position;
    char current_char = lexer_step(lexer);

    switch (current_char) {
        case '.':
            token.type = TT_DOT;
            break;

        case ',':
            token.type = TT_COMMA;
            break;

        case ':':
            token.type = TT_COLON;
            break;

        case ';':
            token.type = TT_SEMICOLON;
            break;

        case '(':
            token.type = TT_LPAREN;
            break;

        case ')':
            token.type = TT_RPAREN;
            break;

        case '[':
            token.type = TT_LBRACKET;
            break;

        case ']':
            token.type = TT_RBRACKET;
            break;

        case '{':
            token.type = TT_LBRACE;
            break;

        case '}':
            token.type = TT_RBRACE;
            break;
    }

    TokenPosition right_pos = lexer->position;

    token.left_pos = left_pos;
    token.right_pos = right_pos;

    return token;
}



Lexer create_lexer() {
    return (Lexer) {
        .code = NULL,
        .code_size = 0,
        .cursor = 0,
        .position = (TokenPosition) { 0 },
    };
}

void free_lexer(Lexer* lexer) {
    if (lexer->code != NULL) {
        free(lexer->code);
        lexer->code = NULL;
    }
}

void load_lexer_code(Lexer* lexer, const char* code, size_t size) {
    lexer->code = alloc_copy(size, code);
    assert(lexer->code != NULL && "Failed to allocate lexer code");

    lexer->code_size = size;
}

TokenArray lexer_tokenize(Lexer* lexer) {
    lexer->cursor = 0;
    lexer->position = (TokenPosition) {
        .line = 0,
        .column = 0,
    };

    TokenArray token_array;
    init_token_array(&token_array, 128);

    while (lexer->cursor < lexer->code_size) {
        char current_char = lexer_at(lexer, 0);

        if (is_char_whitespace(current_char)) {
            /* Do nothing */
            lexer_step(lexer);
        } else if (is_char_digit(current_char)) {
            push_token_array(&token_array, lexer_tokenize_number(lexer));
        } else if (is_char_operator(current_char)) {
            push_token_array(&token_array, lexer_tokenize_operator(lexer));
        } else if (current_char == '"') {
            push_token_array(&token_array, lexer_tokenize_string(lexer));
        } else if (is_char_ident_symbol(current_char)) {
            push_token_array(&token_array, lexer_tokenize_ident(lexer));
        } else if (is_char_symbol(current_char)) {
            push_token_array(&token_array, lexer_tokenize_symbol(lexer));
        } else {
            lexer_step(lexer);
        }
    }

    TokenPosition left_pos = lexer->position;

    push_token_array(&token_array, (Token) {
        .type = TT_EOF,
        .value = NULL,

        .left_pos = left_pos,
        .right_pos = left_pos,
    });

    return token_array;
}
