#include <ast.h>
#include <assert.h>
#include <lexer.h>
#include <parser.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        // printf("%ld: %s (%s)\n", parser->cursor, errmsg, TokenTypeNames[token->type]);
        // exit(1);
        parser_set_error(parser, errmsg, token->left_pos);
        return NULL;
    }

    return token;
}

const Token* parser_advance_expect(Parser* parser, TokenType token_type, const char* errmsg) {
    const Token* token = &parser->tokens->tokens[parser->cursor++];

    if (token->type != token_type) {
        // printf("%ld: %s (%s)\n", parser->cursor, errmsg, TokenTypeNames[token->type]);
        // exit(1);
        parser_set_error(parser, errmsg, token->left_pos);
        return NULL;
    }

    return token;
}

void parser_set_error(Parser* parser, const char* errmsg, TokenPosition errpos) {
    // printf("%ld: %s\n", parser->cursor, errmsg);
    // exit(1);
    // parser->error = errmsg;
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
