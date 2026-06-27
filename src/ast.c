#include "lexer.h"
#include "parser.h"
#include <ast.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***
 * Literal
 */
Node create_literal_node(NodePool* np, const Token* token, NodeType node_type) {
    size_t length = (size_t) strlen(token->value) + 1; // `+ 1` accounts for null terminator

    NodeLiteralData lit_data;
    lit_data.value = allocate_and_write_node_pool(np, token->value, length);
    lit_data.size = length;

    Node result;
    result.type = node_type;
    result.pool_ptr = allocate_and_write_node_pool(np, &lit_data, sizeof(NodeLiteralData));

    return result;
}



/***
 * LITERALS
 */
Node parse_literal_term(Parser* parser) {
    const Token* token = parser_advance(parser);

    if (token->type == TT_INTEGER) {
        return create_literal_node(&parser->node_pool, token, NT_INTEGER_LIT);
    } else if (token->type == TT_FLOAT) {
        return create_literal_node(&parser->node_pool, token, NT_FLOAT_LIT);
    } else if (token->type == TT_STRING) {
        return create_literal_node(&parser->node_pool, token, NT_STRING_LIT);
    } else if (token->type == TT_IDENTIFIER) {
        return create_literal_node(&parser->node_pool, token, NT_IDENT_LIT);
    } else if (token->type == TT_LPAREN) {
        Node expr = parse_expr(parser);

        parser_advance_expect(parser, TT_RPAREN, "Expected ')'"); // `)`
        if (parser->error) return (Node) { 0 };

        return expr;
    } else if (token->type  == TT_SEMICOLON) {
        /* Ignore semicolons */
        Node result;
        result.type = NT_NONE;
        result.pool_ptr = 0;

        return result;
    }

    printf("Token Type: %d (%s), at %ld\n", token->type, TokenTypeNames[token->type], parser->cursor);
    assert(false && "Out of token types in parse_literal_term");
}



/***
 * EXPRESSIONS
 */
Node parse_expr(Parser* parser) {
    return parse_array_expr(parser);
}

/***
 * [ (expr) ]
 * [ (expr), (...) ]
 * [ (expr), ]
 * [ (expr), (...), ]
 */
Node parse_array_expr(Parser* parser) {
    const Token* token = parser_at(parser);

    if (token->type != TT_LBRACKET) {
        return parse_assign_expr(parser);
    }

    parser_advance(parser); // `[`

    // TODO: Resizable node size
    Node* nodes = (Node*) malloc(64 * sizeof(Node));
    size_t nodes_c = 0;

    while (true) {
        token = parser_at(parser);
        if (token->type == TT_RBRACKET) {
            parser_advance(parser); // ']'
            break;
        }

        Node expr = parse_expr(parser);
        if (parser->error) goto error;

        nodes[nodes_c++] = expr;

        token = parser_at(parser);
        if (token->type == TT_COMMA) {
            parser_advance(parser); // ','
            continue;
        } else if (token->type == TT_RBRACKET) {
            parser_advance(parser); // ']'
            break;
        } else {
            parser_set_error(parser, "Expected ',' or ']'", token->left_pos);
        }
    }

    NodeArrayExprData array_data = (NodeArrayExprData) {
        .nodes = allocate_and_write_node_pool(&parser->node_pool, nodes, nodes_c * sizeof(Node)),
        .count = nodes_c,
    };

    Node array_node = (Node) {
        .type = NT_ARRAY_EXPR,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &array_data, 64 * sizeof(Node)),
    };

    return array_node;

    error:
    free(nodes);
    nodes = NULL;

    return (Node) { 0 };
}

/***
 * (ident) = value;
 */
Node parse_assign_expr(Parser* parser) {
    Node left = parse_bin_comp_expr(parser);
    if (parser->error) return (Node) { 0 };

    const Token* token = parser_at(parser);

    if (
        token->type != TT_EQUAL
        && token->type != TT_PLUS_EQUAL
        && token->type != TT_MINUS_EQUAL
        && token->type != TT_TIMES_EQUAL
        && token->type != TT_DIVIDE_EQUAL
        && token->type != TT_MODULO_EQUAL
    ) {
        return left;
    }
    TokenType op = parser_advance(parser)->type; // `=`|`+=`|`-=`|`*=`|`/=`|`%=`

    NodeAssignExprData assign_expr_data = (NodeAssignExprData) {
        .op = op,
        .ident = left,
        // Change parse_bin_comp_expr to parse_assign_expr to allow expr-s like `x = y = z`
        .value = parse_bin_comp_expr(parser),
    };

    left = (Node) {
        .type = NT_ASSIGN_EXPR,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &assign_expr_data, sizeof(NodeAssignExprData)),
    };

    return left;
}

/***
 * (expr) < (expr)
 * (expr) > (expr)
 * (expr) == (expr)
 * (expr) != (expr)
 * (expr) <= (expr)
 * (expr) >= (expr)
 */
Node parse_bin_comp_expr(Parser* parser) {
    Node left = parse_bin_add_expr(parser);
    if (parser->error) return (Node) { 0 };

    // Unwanted tokens non-correlative to the syntax
    if (left.type == NT_NONE) {
        return left;
    }

    while (true) {
        const Token* token = parser_at(parser);

        if (
            token->type != TT_LESS
            && token->type != TT_GREATER
            && token->type != TT_EQUALS
            && token->type != TT_NOT_EQUALS
            && token->type != TT_LESS_EQUALS
            && token->type != TT_GREATER_EQUALS
        ) {
            break;
        }
        parser_advance(parser); // `<`|`>`|`==`|`!=`|`<=`|`>=`

        NodeBinExprData binexpr_data;
        binexpr_data.op = token->type;
        binexpr_data.left = left;
        binexpr_data.right = parse_bin_add_expr(parser);
        if (parser->error) return (Node) { 0 };

        left = (Node) { .type = NT_BIN_EXPR };
        left.pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &binexpr_data, sizeof(NodeBinExprData));
    }

    return left;
}

/***
 * (expr) + (expr)
 * (expr) - (expr)
 */
Node parse_bin_add_expr(Parser* parser) {
    Node left = parse_bin_mul_expr(parser);
    if (parser->error) return (Node) { 0 };

    // Unwanted tokens non-correlative to the syntax
    if (left.type == NT_NONE) {
        return left;
    }

    while (true) {
        const Token* token = parser_at(parser);

        if (token->type != TT_PLUS && token->type != TT_MINUS) {
            break;
        }
        parser_advance(parser); // `+`|`-`

        NodeBinExprData binexpr_data = (NodeBinExprData) {
            .op = token->type,
            .left = left,
            .right = parse_bin_mul_expr(parser),
        };
        if (parser->error) return (Node) { 0 };

        left = (Node) {
            .type = NT_BIN_EXPR,
            .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &binexpr_data, sizeof(NodeBinExprData)),
        };
    }

    return left;
}

/***
 * (expr) * (expr)
 * (expr) / (expr)
 * (expr) % (expr)
 */
Node parse_bin_mul_expr(Parser* parser) {
    Node left = parse_unary_expr(parser);
    if (parser->error) return (Node) { 0 };

    // Unwanted tokens non-correlative to the syntax
    if (left.type == NT_NONE) {
        return left;
    }

    while (true) {
        const Token* token = parser_at(parser);

        if (token->type != TT_ASTERISK && token->type != TT_SLASH && token->type != TT_MODULO) {
            break;
        }
        parser_advance(parser); // `*`|`/`|`%`

        NodeBinExprData binexpr_data = (NodeBinExprData) {
            binexpr_data.op = token->type,
            binexpr_data.left = left,
            binexpr_data.right = parse_unary_expr(parser),
        };
        if (parser->error) return (Node) { 0 };

        left = (Node) {
            .type = NT_BIN_EXPR,
            .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &binexpr_data, sizeof(NodeBinExprData)),
        };
    }

    return left;
}

/***
 * -(expr)
 * *(expr)
 * !(expr)
 * &(expr)
 */
Node parse_unary_expr(Parser* parser) {
    const Token* token = parser_at(parser);

    if (
        token->type != TT_MINUS
        && token->type != TT_ASTERISK
        && token->type != TT_NOT
        && token->type != TT_AMPERSAND
    ) {
        return parse_update_expr(parser);
        if (parser->error) return (Node) { 0 };
    }
    parser_advance(parser); // `-`|`*`|`!`|`&`

    Node left = parse_update_expr(parser);
    if (parser->error) return (Node) { 0 };

    NodeUnaryExprData unaryexpr_data = (NodeUnaryExprData) {
        .op = token->type,
        .expr = left,
    };

    left = (Node) {
        .type = NT_UNARY_EXPR,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &unaryexpr_data, sizeof(NodeUnaryExprData)),
    };

    return left;
}

/***
 * ++(expr)
 * --(expr)
 * (expr)++
 * (expr)--
 */
Node parse_update_expr(Parser* parser) {
    const Token* token = parser_at(parser);

    Node expr = {0};

    // printf("%s\n", TokenTypeNames[parser_at(parser)->type]);

    if (token->type != TT_INCREMENT && token->type != TT_DECREMENT) {
        expr = parse_call_expr(parser);
        if (parser->error) return (Node) { 0 };

        // TODO: Fix update expr potentially grabbing unwanted tokens
        if (expr.type == NT_NONE) {
            return expr;
        }

        token = parser_at(parser);

        /* right-hand side operator */
        if (token->type != TT_INCREMENT && token->type != TT_DECREMENT) {
            return expr;
        }
        parser_advance(parser); // `++`|`--`

        NodeUpdateExprData updateexpr_data = (NodeUpdateExprData) {
            .op = token->type,
            .prefix = false,
            .expr = expr,
        };

        expr = (Node) {
            .type = NT_UPDATE_EXPR,
            .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &updateexpr_data, sizeof(NodeUpdateExprData)),
        };

        return expr;
    } else {
        /* left-hand side operator */
        parser_advance(parser); // `++`|`--`

        expr = parse_call_expr(parser);
        if (parser->error) return (Node) { 0 };

        NodeUpdateExprData updateexpr_data = (NodeUpdateExprData) {
            .op = token->type,
            .prefix = true,
            .expr = expr,
        };

        expr = (Node) {
            .type = NT_UPDATE_EXPR,
            .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &updateexpr_data, sizeof(NodeUpdateExprData)),
        };
    }

    return expr;
}

/***
 * (ident)(args);
 * (ident)(args)(args)(...);
 */
Node parse_call_expr(Parser* parser) {
    Node member = parse_member_expr(parser);
    if (parser->error) return (Node) { 0 };

    // Unwanted tokens non-correlative to the syntax
    if (member.type == NT_NONE) {
        return member;
    }

    while (true) {
        TokenType token_type = parser_at(parser)->type;
        if (token_type != TT_LPAREN) {
            break;
        }
        Node args = parse_args(parser);
        if (parser->error) return (Node) { 0 };

        NodeCallExprData call_expr_data = (NodeCallExprData) {
            .member = member,
            .args = args,
        };

        member = (Node) {
            .type = NT_CALL_EXPR,
            .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &call_expr_data, sizeof(NodeCallExprData)),
        };
    }

    return member;
}

/***
 * (member).(member);
 * (member).(member).(...);
 */
Node parse_member_expr(Parser* parser) {
    Node object = parse_literal_term(parser);
    if (parser->error) return (Node) { 0 };

    // Unwanted tokens non-correlative to the syntax
    if (object.type == NT_NONE) {
        return object;
    }

    while (true) {
        TokenType token_type = parser_at(parser)->type;
        if (token_type != TT_DOT) {
            break;
        }
        parser_advance(parser); // '.'

        TokenPosition property_pos = parser_at(parser)->left_pos;
        Node property = parse_literal_term(parser);
        if (parser->error) return (Node) { 0 };

        if (property.type != NT_IDENT_LIT) {
            parser_set_error(parser, "Expected identifier as member property.", property_pos);
            if (parser->error) return (Node) { 0 };
        }

        NodeMemberExprData member_expr_data = (NodeMemberExprData) {
            .object = object,
            .property = property,
        };

        object = (Node) {
            .type = NT_MEMBER_EXPR,
            .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &member_expr_data, sizeof(NodeMemberExprData)),
        };
    }

    return object;
}



/***
 * { }
 * { (ident): (type) }
 * { (ident): (type), (...) }
 */
Node parse_fields(Parser* parser) {
    parser_advance_expect(parser, TT_LBRACE, "Field list expected '{'"); // `{`
    if (parser->error) return (Node) { 0 }; // We have not allocated anything yet, returning instantly

    // TODO: Resizable node size
    Node* nodes = (Node*) malloc(64 * sizeof(Node));
    size_t nodes_c = 0;

    while (true) {
        const Token* token = parser_at(parser);
        TokenType token_type = token->type; // `,`|`}`

        if (token_type == TT_RBRACE) {
            parser_advance(parser);
            break;
        } else if (token_type == TT_COMMA) {
            parser_advance(parser);

            /* comma before right brace */
            if (parser_at(parser)->type == TT_RBRACE) {
                parser_advance(parser);
                break;
            }
        }

        // printf("%ld %s\n", parser->cursor, TokenTypeNames[token_type]);

        Node name = parse_expr(parser);
        if (parser->error) goto error;

        parser_advance_expect(parser, TT_COLON, "Expected ':'"); // `:`
        if (parser->error) goto error;

        Node type = parse_type(parser);
        if (parser->error) goto error;

        NodeFieldData field_data = (NodeFieldData) {
            .name = name,
            .type = type,
        };

        Node field_node = (Node) {
            .type = NT_FIELD,
            .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &field_data, sizeof(NodeFieldData)),
        };

        nodes[nodes_c++] = field_node;

        token = parser_at(parser);
        token_type = token->type; // `,`|`}`
        if (token_type != TT_RBRACE && token_type != TT_COMMA) {
            parser_set_error(parser, "Expected ',' or '}'", token->left_pos);
            if (parser->error) goto error;
        }
    }

    NodeFieldListData field_list_data = (NodeFieldListData) {
        .nodes = allocate_and_write_node_pool(&parser->node_pool, nodes, nodes_c * sizeof(Node)),
        .count = nodes_c,
    };

    Node field_list_node = (Node) {
        .type = NT_FIELD_LIST,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &field_list_data, sizeof(NodeFieldListData)),
    };

    free(nodes);
    nodes = NULL;

    return field_list_node;

    error:
    free(nodes);
    nodes = NULL;

    return (Node) { 0 };
}

/***
 * ((ident): (type))
 * ((ident): (type), (...))
 */
Node parse_parameters(Parser* parser) {
    parser_advance_expect(parser, TT_LPAREN, "Parameter list expected '('"); // `(`
    if (parser->error) return (Node) { 0 }; // We have not allocated anything yet, returning instantly

    // TODO: Resizable node size
    Node* nodes = (Node*) malloc(64 * sizeof(Node));
    size_t nodes_c = 0;

    while (true) {
        const Token* token = parser_at(parser);
        TokenType token_type = token->type;

        if (token_type == TT_RPAREN) {
            parser_advance(parser); // ')'
            break;
        }

        Node name = parse_expr(parser);
        if (parser->error) goto error;

        parser_advance_expect(parser, TT_COLON, "Expected ':'"); // `:`
        if (parser->error) goto error;

        Node type = parse_type(parser);
        if (parser->error) goto error;

        NodeParamData param_data = (NodeParamData) {
            .name = name,
            .type = type,
        };

        Node param_node = (Node) {
            .type = NT_PARAM,
            .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &param_data, sizeof(NodeParamData)),
        };

        nodes[nodes_c++] = param_node;
        // printf("%ld %d %d\n", nodes_c, param_node.type, (uintptr_t) param_node.pool_ptr);

        token = parser_advance(parser);
        token_type = token->type; // `,`|`)`

        if (token_type == TT_RPAREN) {
            break;
        } else if (token_type != TT_COMMA) {
            parser_set_error(parser, "Expected ','", token->left_pos);
            if (parser->error) goto error;
        }
    }

    NodeParamListData param_list_data = (NodeParamListData) {
        .nodes = allocate_and_write_node_pool(&parser->node_pool, nodes, nodes_c * sizeof(Node)),
        .count = nodes_c,
    };

    Node param_list_node = (Node) {
        .type = NT_PARAM_LIST,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &param_list_data, sizeof(NodeParamListData)),
    };

    free(nodes);
    nodes = NULL;

    return param_list_node;

    error:
    free(nodes);
    nodes = NULL;

    return (Node) { 0 };
}

/***
 * { (ident) }
 * { (ident), (...) }
 * { (ident) = (expr) }
 * { (ident) = (expr), (...) }
 */
Node parse_enum_entries(Parser* parser) {
    parser_advance_expect(parser, TT_LBRACE, "Enum entries list expected '{'"); // '{'
    if (parser->error) return (Node) { 0 }; // We have not allocated anything yet, returning instantly

    // TODO: Resizable node size
    Node* nodes = (Node*) malloc(64 * sizeof(Node));
    size_t nodes_c = 0;

    while (true) {
        const Token* token = parser_at(parser);

        if (token->type == TT_RBRACE) {
            parser_advance(parser);
            break;
        } else if (token->type == TT_COMMA) {
            parser_advance(parser);
        }

        token = parser_at(parser);

        if (token->type == TT_RBRACE) {
            parser_advance(parser);
            break;
        }

        TokenPosition token_pos = parser_at(parser)->left_pos;
        Node ident = parse_literal_term(parser);
        if (parser->error) goto error;

        if (ident.type != NT_IDENT_LIT) {
            parser_set_error(parser, "Expected identifier as Enumeration Entry", token_pos);
            goto error;
        }

        Node value = (Node) { 0 };

        token = parser_at(parser);

        if (token->type == TT_EQUAL) {
            parser_advance(parser); // '='

            value = parse_expr(parser);
            if (parser->error) goto error;
        }

        NodeEnumEntryData entry_data = (NodeEnumEntryData) {
            .name = ident,
            .value = value,
        };

        Node entry_node = (Node) {
            .type = NT_ENUM_ENTRY,
            .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &entry_data, sizeof(NodeEnumEntryData)),
        };

        nodes[nodes_c++] = entry_node;

        token = parser_at(parser);

        if (token->type != TT_RBRACE && token->type != TT_COMMA) {
            parser_set_error(parser, "Expected ',' or '}'", token->left_pos);
            if (parser->error) goto error;
        }
    }

    NodeEnumEntryListData arg_list_data = (NodeEnumEntryListData) {
        .nodes = allocate_and_write_node_pool(&parser->node_pool, nodes, nodes_c * sizeof(Node)),
        .count = nodes_c,
    };

    Node entry_list_node = (Node) {
        .type = NT_ENUM_ENTRY_LIST,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &arg_list_data, sizeof(NodeEnumEntryListData)),
    };

    free(nodes);
    nodes = NULL;

    return entry_list_node;

    error:
    free(nodes);
    nodes = NULL;

    return (Node) { 0 };
}

/***
 * ((expr))
 * ((expr), (expr), (...))
 */
Node parse_args(Parser* parser) {
    parser_advance_expect(parser, TT_LPAREN, "Parameter list expected '('"); // `(`
    if (parser->error) return (Node) { 0 }; // We have not allocated anything yet, returning instantly

    // TODO: Resizable node size
    Node* nodes = (Node*) malloc(64 * sizeof(Node));
    size_t nodes_c = 0;

    while (true) {
        const Token* token = parser_at(parser);
        TokenType token_type = token->type;

        if (token_type == TT_RPAREN) {
            parser_advance(parser);
            break;
        } else if (token_type == TT_COMMA) {
            parser_advance(parser);
        }

        Node expr = parse_expr(parser);
        if (parser->error) goto error;

        NodeArgData arg_data = (NodeArgData) {
            .expr = expr,
        };

        Node arg_node = (Node) {
            .type = NT_ARG,
            .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &arg_data, sizeof(NodeArgData)),
        };

        nodes[nodes_c++] = arg_node;

        token = parser_at(parser);
        token_type = token->type; // `,`|`)`

        if (token_type != TT_RPAREN && token_type != TT_COMMA) {
            parser_set_error(parser, "Expected ',' or ')'", token->left_pos);
            if (parser->error) goto error;
        }
    }

    NodeArgListData arg_list_data = (NodeArgListData) {
        .nodes = allocate_and_write_node_pool(&parser->node_pool, nodes, nodes_c * sizeof(Node)),
        .count = nodes_c,
    };

    Node param_list_node = (Node) {
        .type = NT_PARAM_LIST,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &arg_list_data, sizeof(NodeArgListData)),
    };

    free(nodes);
    nodes = NULL;

    return param_list_node;

    error:
    free(nodes);
    nodes = NULL;

    return (Node) { 0 };
}

/***
 * { (...) }
 */
Node parse_block(Parser* parser) {
    parser_advance(parser); // `{`

    // TODO: Resizable node size
    Node* nodes = (Node*) malloc(64 * sizeof(Node));
    size_t nodes_c = 0;

    while (parser->cursor < parser->tokens->count) {
        if (parser_at(parser)->type == TT_RBRACE) {
            break;
        }

        Node node = parse_stat(parser);
        if (parser->error) goto error;

        /* Skip ignored nodes */
        if (node.type == NT_NONE) {
            continue;
        }

        nodes[nodes_c++] = node;
    }

    // parser_advance_expect(parser, TT_RBRACE, "Expected '}'"); // `}`
    parser_advance(parser); // '}'

    /* Copying the node array buffer into the node pool, and then freeing the node array buffer */
    NodeBlockData block_data;
    block_data.nodes = allocate_and_write_node_pool(&parser->node_pool, nodes, nodes_c * sizeof(Node));
    block_data.count = nodes_c;

    free(nodes);
    nodes = NULL;

    Node result = (Node) { .type = NT_BLOCK_EXPR };
    result.pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &block_data, sizeof(NodeBlockData));

    return result;

    error:
    free(nodes);
    nodes = NULL;

    return (Node) { 0 };
}

/***
 * (expr)
 * (array type)
 */
Node parse_type(Parser* parser) {
    if (parser_at(parser)->type == TT_LBRACKET) {
        return parse_array_type(parser);
    }

    return parse_member_expr(parser);
}

/***
 * [](expr)
 * (...)[][](expr)
 * [(expr)](expr)
 * (...)[(expr)][(expr)](expr)
 */
Node parse_array_type(Parser* parser) {
    const Token* token = NULL;
    NodeArrayTypeData array_type_data = { 0 };

    // TODO: Remove while loop
    while (true) {
        token = parser_at(parser);

        if (token->type == TT_LBRACKET) {
            parser_advance(parser); // '['
        }

        token = parser_at(parser);

        // ArrayType without size
        if (token->type == TT_RBRACKET) {
            parser_advance(parser); // ']'

            // [](expr)
            array_type_data.type = parse_type(parser);
            if (parser->error) return (Node) { 0 };

            break;
        }

        // [(expr)]
        array_type_data.size = parse_expr(parser);
        if (parser->error) return (Node) { 0 };

        token = parser_at(parser);

        // Array type with size
        if (token->type == TT_RBRACKET) {
            parser_advance(parser); // ']'

            // [](expr)
            array_type_data.type = parse_type(parser);
            if (parser->error) return (Node) { 0 };
        }

        break;
    }

    Node array_type_node = (Node) {
        .type = NT_ARRAY_TYPE,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &array_type_data, sizeof(NodeArrayTypeData)),
    };

    return array_type_node;
}

/***
 * if ...
 * else if (expr) (block)
 * else (expr|block)
 */
Node parse_if_else(Parser* parser) {
    parser_advance(parser); // `else`

    const Token* token = parser_at(parser);

    if (token->type == TT_IF) {
        return parse_if_stat(parser);
    } else if (token->type == TT_LBRACE) {
        return parse_block(parser);
    }

    return parse_expr(parser);
}

Node parse_switch_block(Parser* parser) {
    parser_advance(parser); // '{'

    // TODO: Resizable node size
    Node* nodes = malloc(64 * sizeof(Node));
    size_t nodes_c = 0;

    bool has_default = false;

    while (true) {
        const Token* token = parser_at(parser);

        if (token->type == TT_RBRACE) {
            parser_advance(parser); // '}'
            break;
        } else if (token->type == TT_CASE) {
            parser_advance(parser); // 'case'

            Node expr = parse_expr(parser);
            if (parser->error) goto error;

            parser_advance_expect(parser, TT_COLON, "Expected ':'");
            if (parser->error) goto error;

            Node block = parse_block(parser);
            if (parser->error) goto error;

            NodeSwitchEntryData entry_data = (NodeSwitchEntryData) {
                .is_default = false,
                .expr = expr,
                .block = block,
            };

            Node entry_node = (Node) {
                .type = NT_SWITCH_ENTRY,
                .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &entry_data, sizeof(NodeSwitchEntryData)),
            };

            nodes[nodes_c++] = entry_node;
        } else if (token->type == TT_DEFAULT) {
            token = parser_advance(parser); // 'default'
            // printf("%s\n", TokenTypeNames[token->type]);

            if (has_default) {
                parser_set_error(parser, "Multiple default cases in one switch statement", token->left_pos);
                goto error;
            }
            has_default = true;

            parser_advance_expect(parser, TT_COLON, "Expected ':'");
            if (parser->error) goto error;

            Node block = parse_block(parser);
            if (parser->error) goto error;

            NodeSwitchEntryData entry_data = (NodeSwitchEntryData) {
                .is_default = true,
                .expr = (Node) { 0 },
                .block = block,
            };

            Node entry_node = (Node) {
                .type = NT_SWITCH_ENTRY,
                .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &entry_data, sizeof(NodeSwitchEntryData)),
            };

            nodes[nodes_c++] = entry_node;
        }
    }

    NodeSwitchEntryListData switch_entry_list_data = (NodeSwitchEntryListData) {
        .entries = allocate_and_write_node_pool(&parser->node_pool, nodes, nodes_c * sizeof(NodeSwitchEntryData)),
        .count = nodes_c,
    };

    Node switch_entry_list_node = (Node) {
        .type = NT_SWITCH_ENTRY_LIST,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &switch_entry_list_data, sizeof(NodeSwitchEntryListData)),
    };

    free(nodes);
    nodes = NULL;

    return switch_entry_list_node;

    error:
    free(nodes);
    nodes = NULL;

    return (Node) { 0 };
}



/***
 * STATEMENTS
 */
Node parse_stat(Parser* parser) {
    const Token* token = parser_at(parser);

    if (token->type == TT_IF) {
        return parse_if_stat(parser);
    } else if (token->type == TT_WHILE) {
        return parse_while_stat(parser);
    } else if (token->type == TT_SWITCH) {
        return parse_switch_stat(parser);
    } else if (token->type == TT_FN) {
        return parse_fn_stat(parser);
    } else if (token->type == TT_ENUM) {
        return parse_enum_stat(parser);
    } else if (token->type == TT_STRUCT) {
        return parse_struct_stat(parser);
    } else if (token->type == TT_VAR || token->type == TT_CONST) {
        return parse_var_stat(parser);
    } else if (token->type == TT_RETURN) {
        return parse_return_stat(parser);
    } else if (token->type == TT_BREAK) {
        parser_advance(parser); // 'break'

        Node node = (Node) {
            .type = NT_BREAK_STAT,
            .pool_ptr = NULL,
        };

        return node;
    } else if (token->type == TT_CONTINUE) {
        parser_advance(parser); // 'continue'

        Node node = (Node) {
            .type = NT_CONTINUE_STAT,
            .pool_ptr = NULL,
        };

        return node;
    } else if (token->type == TT_LBRACE) {
        return parse_block(parser);
    }

    return parse_expr(parser);
}

/***
 * return;
 * return (value);
 */
Node parse_return_stat(Parser* parser) {
    parser_advance(parser); // `return`

    Node value = parse_expr(parser);
    if (parser->error) return (Node) { 0 };

    NodeReturnStatData return_data = (NodeReturnStatData) {
        .value = value,
    };

    Node result = (Node) {
        .type = NT_RETURN_STAT,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &return_data, sizeof(NodeReturnStatData)),
    };

    return result;
}

/***
 * var|const (indent): (type);
 * var|const (indent): (type) = (value);
 */
Node parse_var_stat(Parser* parser) {
    TokenType token_type = parser_advance(parser)->type; // `var`|`const`
    bool is_const = token_type == TT_CONST;

    Node ident = { 0 };
    Node type = { 0 };
    Node value = { 0 };

    TokenPosition token_pos = parser_at(parser)->left_pos;
    ident = parse_literal_term(parser);
    if (parser->error) return (Node) { 0 };

    if (ident.type != NT_IDENT_LIT) {
        parser_set_error(parser, "Expected identifier for variable statement name", token_pos);
        return (Node) { 0 };
    }

    parser_advance_expect(parser, TT_COLON, "VarStat: Expected ':'"); // `:`
    if (parser->error) return (Node) { 0 };

    type = parse_type(parser);
    if (parser->error) return (Node) { 0 };

    TokenType equal_token_type = parser_at(parser)->type; // `=`

    if (equal_token_type == TT_EQUAL) {
        parser_advance(parser); // `=`
        value = parse_expr(parser);
    }

    NodeVarStatData var_data = (NodeVarStatData) {
        .is_const = is_const,
        .ident = ident,
        .type = type,
        .value = value,
    };

    Node result = (Node) {
        .type = NT_VAR_STAT,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &var_data, sizeof(NodeVarStatData)),
    };

    return result;
}

/***
 * enum (ident)
 * enum (ident) { (enum entries) }
 */
Node parse_enum_stat(Parser* parser) {
    parser_advance(parser); // 'enum'

    TokenPosition token_pos = parser_at(parser)->left_pos;

    // Avoid conflict with call expr
    Node ident = parse_literal_term(parser);
    if (parser->error) return (Node) { 0 };

    if (ident.type != NT_IDENT_LIT) {
        parser_set_error(parser, "Expected identifier as struct name", token_pos);
        return (Node) { 0 };
    }

    Node entries = { 0 };

    TokenType token_type = parser_at(parser)->type;

    if (token_type == TT_LBRACE) {
        entries = parse_enum_entries(parser);
        if (parser->error) return (Node) { 0 };
    }

    NodeEnumData enum_data = (NodeEnumData) {
        .ident = ident,
        .entries = entries,
    };

    Node enum_node = (Node) {
        .type = NT_ENUM_STAT,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &enum_data, sizeof(NodeEnumData)),
    };

    return enum_node;
}

/***
 * struct (ident);
 * struct (ident) (fields)
 */
Node parse_struct_stat(Parser* parser) {
    parser_advance(parser); // `struct`

    TokenPosition token_pos = parser_at(parser)->left_pos;

    // Avoid conflict with call expr
    Node ident = parse_literal_term(parser);
    if (parser->error) return (Node) { 0 };

    if (ident.type != NT_IDENT_LIT) {
        parser_set_error(parser, "Expected identifier as struct name", token_pos);
        return (Node) { 0 };
    }

    Node fields = { 0 };

    TokenType token_type = parser_at(parser)->type;

    if (token_type == TT_LBRACE) {
        fields = parse_fields(parser);
        if (parser->error) return (Node) { 0 };
    }

    NodeStructData struct_data = (NodeStructData) {
        .ident = ident,
        .fields = fields,
    };

    Node result = (Node) {
        .type = NT_STRUCT_STAT,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &struct_data, sizeof(NodeStructData)),
    };

    return result;
}

/***
 * fn (ident) (params): (type) (block)
 */
Node parse_fn_stat(Parser* parser) {
    parser_advance(parser); // `fn`
    // printf("%s\n", TokenTypeNames[parser_at(parser)->type]);

    TokenPosition token_pos = parser_at(parser)->left_pos;

    // Avoid conflict with call expr
    Node ident = parse_literal_term(parser);
    if (parser->error) return (Node) { 0 };

    if (ident.type != NT_IDENT_LIT) {
        parser_set_error(parser, "Expected identifier as function name", token_pos);
        return (Node) { 0 };
    }

    // printf("%s\n", TokenTypeNames[parser_at(parser)->type]);

    Node params = parse_parameters(parser);
    if (parser->error) return (Node) { 0 };

    parser_advance_expect(parser, TT_COLON, "Expected ':'"); // `:`
    if (parser->error) return (Node) { 0 };

    Node type = parse_type(parser);
    if (parser->error) return (Node) { 0 };

    parser_at_expect(parser, TT_LBRACE, "Expected '{'"); // `{`
    if (parser->error) return (Node) { 0 };

    Node block = parse_block(parser);
    if (parser->error) return (Node) { 0 };

    NodeFunctionData func_data = (NodeFunctionData) {
        .ident = ident,
        .params = params,
        .type = type,
        .block = block,
    };

    Node result = (Node) {
        .type = NT_FUNCTION_STAT,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &func_data, sizeof(NodeFunctionData)),
    };

    return result;
}

/***
 * if (expr) (block)
 */
Node parse_if_stat(Parser* parser) {
    const Token* token = NULL;

    parser_advance(parser); // `if`

    parser_advance_expect(parser, TT_LPAREN, "The expression in if statement must be closed in parentheses ('(')"); // `(`
    if (parser->error) return (Node) { 0 };

    Node expr = parse_expr(parser);
    if (parser->error) return (Node) { 0 };

    parser_advance_expect(parser, TT_RPAREN, "The expression in if statement must be closed in parentheses (')')"); // `)`
    if (parser->error) return (Node) { 0 };

    parser_at_expect(parser, TT_LBRACE, "Expected '{'"); // `{`
    if (parser->error) return (Node) { 0 };

    /* ---------------- */
    Node block = (Node) { 0 };
    token = parser_at(parser);

    if (token->type == TT_LBRACE) {
        block = parse_block(parser);
    } else {
        block = parse_expr(parser);
    }

    if (parser->error) return (Node) { 0 };
    /* ---------------- */

    /* ---------------- */
    Node ifelse = (Node) { 0 };
    token = parser_at(parser);

    if (token->type == TT_ELSE) {
        // parser_advance(parser); // `else`

        ifelse = parse_if_else(parser);
        if (parser->error) return (Node) { 0 };
    }
    /* ---------------- */

    NodeIfStatData if_data;
    if_data.expr = expr;
    if_data.block = block;
    if_data.ifelse = ifelse;

    Node result = (Node) { .type = NT_IF_STAT };
    result.pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &if_data, sizeof(NodeIfStatData));

    return result;
}

/***
 * while (expr) (block)
 */
Node parse_while_stat(Parser* parser) {
    parser_advance(parser); // `while`

    parser_advance_expect(parser, TT_LPAREN, "The expression in while statement must be closed in parentheses ('(')"); // `(`
    if (parser->error) return (Node) { 0 };

    Node expr = parse_expr(parser);
    if (parser->error) return (Node) { 0 };

    parser_advance_expect(parser, TT_RPAREN, "The expression in while statement must be closed in parentheses (')')"); // `)`
    if (parser->error) return (Node) { 0 };

    // parser_at_expect(parser, TT_LBRACE, "Expected '{'"); // `{`
    // if (parser->error) return (Node) { 0 };

    /* ---------------- */
    Node block = (Node) { 0 };
    const Token* token = parser_at(parser);

    if (token->type == TT_LBRACE) {
        block = parse_block(parser);
    } else {
        block = parse_expr(parser);
    }

    if (parser->error) return (Node) { 0 };
    /* ---------------- */

    NodeWhileData while_data;
    while_data.expr = expr;
    while_data.block = block;

    Node result = (Node) { .type = NT_WHILE_STAT };
    result.pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &while_data, sizeof(NodeWhileData));

    return result;
}

/***
 * switch (expr) {}
 * switch (expr) { case (expr): (block), (...) }
 * switch (expr) { default: (block) }
 */
Node parse_switch_stat(Parser* parser) {
    parser_advance(parser); // 'switch'

    parser_advance_expect(parser, TT_LPAREN, "Expected '(' in switch statement");
    if (parser->error) return (Node) { 0 };

    Node lookup = parse_expr(parser);
    if (parser->error) return (Node) { 0 };

    parser_advance_expect(parser, TT_RPAREN, "Expected ')' in switch statement");
    if (parser->error) return (Node) { 0 };

    Node block = parse_switch_block(parser);
    if (parser->error) return (Node) { 0 };

    NodeSwitchData switch_data = (NodeSwitchData) {
        .lookup = lookup,
        .block = block,
    };

    Node switch_node = (Node) {
        .type = NT_SWITCH_STAT,
        .pool_ptr = allocate_and_write_node_pool(&parser->node_pool, &switch_data, sizeof(NodeSwitchData)),
    };

    return switch_node;
}
