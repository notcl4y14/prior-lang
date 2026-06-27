#include <ast.h>
#include <assert.h>
#include <lexer.h>
#include <parser.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* NodeTypeNames[] = {
    [NT_NONE] = "NONE",

    [NT_PROGRAM] = "PROGRAM",

    [NT_RETURN_STAT]   = "RETURN_STAT",
    [NT_BREAK_STAT]    = "BREAK_STAT",
    [NT_CONTINUE_STAT] = "CONTINUE_STAT",
    [NT_VAR_STAT]      = "VAR_STAT",
    [NT_ENUM_STAT]     = "ENUM_STAT",
    [NT_STRUCT_STAT]   = "STRUCT_STAT",
    [NT_FUNCTION_STAT] = "FUNCTION_STAT",
    [NT_IF_STAT]       = "IF_STAT",
    [NT_WHILE_STAT]    = "WHILE_STAT",
    [NT_SWITCH_STAT]   = "SWITCH_STAT",

    [NT_ARRAY_EXPR]  = "ARRAY_EXPR",
    [NT_BIN_EXPR]    = "BIN_EXPR",
    [NT_UNARY_EXPR]  = "UNARY_EXPR",
    [NT_UPDATE_EXPR] = "UPDATE_EXPR",
    [NT_TERN_EXPR]   = "TERN_EXPR",
    [NT_ASSIGN_EXPR] = "ASSIGN_EXPR",
    [NT_CALL_EXPR]   = "CALL_EXPR",
    [NT_MEMBER_EXPR] = "MEMBER_EXPR",

    [NT_BLOCK_EXPR]        = "BLOCK",
    [NT_PARAMLIST]         = "PARAM_LIST",
    [NT_PARAM]             = "PARAM",
    [NT_ENUM_ENTRY_LIST]   = "ENUM_ENTRY_LIST",
    [NT_ENUM_ENTRY]        = "ENUM_ENTRY",
    [NT_ARGLIST]           = "ARG_LIST",
    [NT_ARG]               = "ARG",
    [NT_FIELDLIST]         = "FIELD_LIST",
    [NT_FIELD]             = "FIELD",
    [NT_ARRAY_TYPE]        = "ARRAY_TYPE",
    [NT_SWITCH_ENTRY_LIST] = "SWITCH_ENTRY_LIST",
    [NT_SWITCH_ENTRY]      = "SWITCH_ENTRY",

    [NT_INTEGER_LIT] = "INTEGER_LIT",
    [NT_FLOAT_LIT]   = "FLOAT_LIT",
    [NT_STRING_LIT]  = "STRING_LIT",
    [NT_IDENT_LIT]   = "IDENT_LIT",
};

Parser create_parser(TokenArray* token_array) {
    return (Parser) {
        .tokens = token_array,
        .cursor = 0,
        .node_pool = (NodePool) { 0 },
        .errmsg = { 0 },
        .error = false,
    };
}

void free_parser(Parser* parser) {
    if (parser->node_pool.data != NULL) {
        free_node_pool(&parser->node_pool);
    }
}

const Token* parser_at(Parser* parser) {
    return &parser->tokens->tokens[parser->cursor];
}

const Token* parser_advance(Parser* parser) {
    return &parser->tokens->tokens[parser->cursor++];
}

const Token* parser_at_expect(Parser* parser, TokenType token_type, const char* errmsg) {
    const Token* token = &parser->tokens->tokens[parser->cursor];

    if (token->type != token_type) {
        parser_set_error(parser, errmsg, token->left_pos);
        return NULL;
    }

    return token;
}

const Token* parser_advance_expect(Parser* parser, TokenType token_type, const char* errmsg) {
    const Token* token = &parser->tokens->tokens[parser->cursor++];

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
    init_node_pool(&parser->node_pool, INITIAL_NODE_POOL_SIZE);

    /* Initializing node pool */
    Node ast = (Node) { .type = NT_PROGRAM };
    ast.pool_ptr = allocate_node_pool(&parser->node_pool, sizeof(NodeBlockData));

    /* Node array buffer */
    // TODO: Resizable node size
    Node* nodes = (Node*) malloc(64 * sizeof(Node));
    size_t nodes_c = 0;

    while (parser->cursor < parser->tokens->count) {
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

        // printf("%ld\n", nodes_c);
        nodes[nodes_c++] = node;
    }

    if (parser->error == true) {
        free(nodes);
        nodes = NULL;

        free_node_pool(&parser->node_pool);

        return (Node) { .type = NT_NONE, .pool_ptr = NULL };
    }

    /* Copying the node array buffer into the node pool, and then freeing the node array buffer */
    NodeBlockData* ast_data = (NodeBlockData*)(ast.pool_ptr);
    ast_data->nodes = allocate_and_write_node_pool(&parser->node_pool, nodes, nodes_c * sizeof(Node));
    ast_data->count = nodes_c;

    free(nodes);
    nodes = NULL;

    return ast;
}
