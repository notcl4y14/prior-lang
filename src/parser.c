#include <ast.h>
#include <assert.h>
#include <lexer.h>
#include <parser.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

const char* NodeTypeNames[] = {
    [NT_NONE] = "None",

    [NT_PROGRAM] = "Program",

    [NT_RETURN_STAT]   = "ReturnStat",
    [NT_BREAK_STAT]    = "BreakStat",
    [NT_CONTINUE_STAT] = "ContinueStat",
    [NT_VAR_STAT]      = "VarStat",
    [NT_ENUM_STAT]     = "EnumStat",
    [NT_STRUCT_STAT]   = "StructStat",
    [NT_FUNC_STAT]     = "FuncStat",
    [NT_IF_STAT]       = "IfStat",
    [NT_WHILE_STAT]    = "WhileStat",
    [NT_SWITCH_STAT]   = "SwitchStat",

    [NT_BIN_EXPR]    = "BinExpr",
    [NT_UN_EXPR]     = "UnExpr",
    [NT_UPDATE_EXPR] = "UpdateExpr",
    [NT_TERN_EXPR]   = "TernExpr",
    [NT_ASSIGN_EXPR] = "AssignExpr",
    [NT_CALL_EXPR]   = "CallExpr",
    [NT_MEMBER_EXPR] = "MemberExpr",

    [NT_BLOCK]       = "Block",
    [NT_PARAMETER]   = "Parameter",
    [NT_ENUM_ENTRY]  = "EnumEntry",
    [NT_ARGUMENT]    = "Argument",
    [NT_FIELD]       = "Field",
    [NT_ARRAY_TYPE]  = "ArrayType",
    [NT_SWITCH_CASE] = "SwitchCase",

    [NT_INTEGER_LIT] = "Integer",
    [NT_FLOAT_LIT]   = "Float",
    [NT_STRING_LIT]  = "String",
    [NT_IDENT_LIT]   = "Ident",
    [NT_ARRAY_LIT]   = "Array",
};



NodeArr create_node_arr(size_t size) {
    NodeArr narr = (NodeArr) { 0 };

    narr.nodes = calloc(size, sizeof(Node));
    assert(narr.nodes != NULL && "Failed to allocate node array");

    narr.count = 0;
    narr.capacity = size;

    return narr;
}

void free_node_arr(NodeArr* narr) {
    free(narr->nodes);
    narr->nodes = NULL;
}

void add_to_node_arr(NodeArr* narr, Node node) {
    if (narr->count >= narr->capacity) {
        narr->capacity *= 2;
        narr->nodes = realloc(narr->nodes, narr->capacity * sizeof(Node));
        assert(narr->nodes != NULL && "Failed to reallocate node array");
    }

    narr->nodes[narr->count++] = node;
}



Parser create_parser(TokenArray token_array) {
    return (Parser) {
        .tokens = token_array,
        .cursor = 0,
        .errmsg = { 0 },
        .error = false,
    };
}

void free_parser(Parser* parser) { }

const Token* parser_at(Parser* parser) {
    return &parser->tokens.tokens[parser->cursor];
}

const Token* parser_advance(Parser* parser) {
    return &parser->tokens.tokens[parser->cursor++];
}

const Token* parser_at_expect(Parser* parser, TokenType token_type, const char* errmsg) {
    const Token* token = &parser->tokens.tokens[parser->cursor];

    if (token->type != token_type) {
        parser_set_error(parser, errmsg, token->left_pos);
        return NULL;
    }

    return token;
}

const Token* parser_advance_expect(Parser* parser, TokenType token_type, const char* errmsg) {
    const Token* token = &parser->tokens.tokens[parser->cursor++];

    if (token->type != token_type) {
        parser_set_error(parser, errmsg, token->left_pos);
        return NULL;
    }

    return token;
}

void parser_set_error(Parser* parser, const char* errmsg, TokenPosition errpos) {
    strncpy(parser->errmsg, errmsg, PARSER_ERROR_SIZE);
    parser->errpos = errpos;
    parser->error = true;
}

const char* parser_get_error(Parser* parser) {
    if (!parser->error) {
        return NULL;
    }

    return parser->errmsg;
}

/***
 * Parsing
 */
Node parse_tokens(Parser* parser) {
    Node ast = (Node) { .type = NT_PROGRAM };

    NodeArr node_array = create_node_arr(64);

    while (parser->cursor < parser->tokens.count) {
        if (parser_at(parser)->type == TT_EOF) {
            break;
        }

        Node node = parse_stat(parser);

        if (parser->error == true) {
            break;
        }

        /* Skip ignored nodes */
        if (node.type == NT_NONE) {
            continue;
        }

        add_to_node_arr(&node_array, node);
    }

    if (parser->error == true) {
        free_node_arr(&node_array);
        return (Node) { .type = NT_NONE, .data = {}, };
    }

    ast.data.program.nodes = node_array;

    return ast;
}
