#include "lexer.h"
#include "mem.h"
#include "parser.h"
#include "token.h"
#include <assert.h>
#include <ast.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node* new_node(const Node* node) {
    Node* allocation = malloc(sizeof(Node));
    *allocation = *node;
    return allocation;
}

/***
 * LITERALS
 */
Node parse_literal_term(Parser* parser) {
    const Token* token = parser_at(parser);

    if (token->type == TT_INTEGER) {
        parser_advance(parser);
        Node node = (Node) { .type = NT_INTEGER_LIT, {} };
        node.data.int_lit.value = str_alloc_copy(token->value);
        node.data.int_lit.size = (size_t) strlen(token->value) + 1;

        node.left_pos = token->left_pos;
        node.right_pos = token->right_pos;

        return node;
    } else if (token->type == TT_FLOAT) {
        parser_advance(parser);
        Node node = (Node) { .type = NT_FLOAT_LIT, {} };
        node.data.int_lit.value = str_alloc_copy(token->value);
        node.data.int_lit.size = (size_t) strlen(token->value) + 1;

        node.left_pos = token->left_pos;
        node.right_pos = token->right_pos;

        return node;
    } else if (token->type == TT_STRING) {
        parser_advance(parser);
        Node node = (Node) { .type = NT_STRING_LIT, {} };
        node.data.int_lit.value = str_alloc_copy(token->value);
        node.data.int_lit.size = (size_t) strlen(token->value) + 1;

        node.left_pos = token->left_pos;
        node.right_pos = token->right_pos;

        return node;
    } else if (token->type == TT_IDENTIFIER) {
        parser_advance(parser);
        Node node = (Node) { .type = NT_IDENT_LIT, {} };
        node.data.int_lit.value = str_alloc_copy(token->value);
        node.data.int_lit.size = (size_t) strlen(token->value) + 1;

        node.left_pos = token->left_pos;
        node.right_pos = token->right_pos;

        return node;
    } else if (token->type == TT_LPAREN) {
        parser_advance(parser);
        /***
         * This is the literal term expression I was talking about
         * in parse_cast_expr.
         */
        Node expr = parse_expr(parser);

        parser_advance_expect(parser, TT_RPAREN, "Expected ')'");
        if (parser->error) return (Node) { 0 };

        return expr;
    } else if (token->type == TT_LBRACKET) {
        Node array_lit = parse_array_lit(parser);
        if (parser->error) return (Node) { 0 };

        return array_lit;
    } else if (token->type == TT_LBRACE) {
        Node compound_lit = parse_compound_literal(parser);
        if (parser->error) return (Node) { 0 };

        return compound_lit;
    } else if (token->type  == TT_SEMICOLON) {
        parser_advance(parser);
        /* Ignore semicolons */
        Node result = (Node) { .type = NT_NONE, {} };

        return result;
    }

    parser_advance(parser);

    char errbuf[256] = { 0 };
    sprintf(errbuf, "Unexpected token %s", TokenTypeNames[token->type]);
    parser_set_error(parser, errbuf, token->left_pos);
    return (Node) { .type = NT_NONE, {} };
}

/***
 * [ (expr) ]
 * [ (expr), (...) ]
 * [ (expr), ]
 * [ (expr), (...), ]
 */
Node parse_array_lit(Parser* parser) {
    const Token* starting_token = parser_advance(parser); // '['
    const Token* ending_token = NULL;

    NodeArr node_array = create_node_arr(64);

    while (true) {
        if (parser_at(parser)->type == TT_RBRACKET) {
            ending_token = parser_advance(parser); // ']'
            break;
        }


        Node expr = parse_expr(parser);
        if (parser->error) goto error;

        add_to_node_arr(&node_array, expr);


        if (parser_at(parser)->type == TT_COMMA) {
            parser_advance(parser); // ','
            continue;
        } else if (parser_at(parser)->type == TT_RBRACKET) {
            ending_token = parser_advance(parser); // ']'
            break;
        } else {
            parser_set_error(parser, "Expected ',' or ']'", parser_at(parser)->left_pos);
        }
    }

    Node array_node = (Node) {
        .type = NT_ARRAY_LIT,
        .data.array_lit = (NArrayLit) {
            .values = node_array,
        },
        .left_pos = starting_token->left_pos,
        .right_pos = ending_token->right_pos,
    };

    return array_node;

    error:
    free_node_arr(&node_array);
    return (Node) { 0 };
}

/***
 * { }
 * { (expr) }
 * { (expr), (expr), ... }
 * { (member) = (expr) }
 * { (member) = (expr), (expr), ... }
 * { (expr), (member) = (expr), ... }
 * { (member) = (expr), (member) = (expr), ... }
 */
Node parse_compound_literal(Parser* parser) {
    const Token* starting_token = parser_advance(parser); // '{'
    const Token* ending_token = NULL;

    NodeArr node_array = create_node_arr(64);

    while (true) {
        if (parser_at(parser)->type == TT_RBRACE) {
            ending_token = parser_advance(parser);
            break;
        }

        // printf("%s\n", TokenTypeNames[parser_at(parser)->type]);

        Node expr = parse_expr(parser);
        if (parser->error) goto error;

        add_to_node_arr(&node_array, expr);

        const Token* token = parser_at(parser);

        if (token->type == TT_COMMA) {
            parser_advance(parser);
        } else if (token->type != TT_RBRACE) {
            parser_set_error(parser, "Expected ','|'}'", token->left_pos);
            goto error;
        }
    }

    Node result = (Node) {
        .type = NT_COMPOUND_LIT,
        .data.compound_lit = (NCompoundLit) {
            .values = node_array,
        },
        .left_pos = starting_token->left_pos,
        .right_pos = ending_token->right_pos,
    };

    return result;

    error:
    free_node_arr(&node_array);
    return (Node) { 0 };
}



/***
 * EXPRESSIONS
 */
Node parse_expr(Parser* parser) {
    return parse_assign_expr(parser);
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

    Node value = parse_bin_comp_expr(parser);
    if (parser->error) return (Node) { 0 };

    left = (Node) {
        .type = NT_ASSIGN_EXPR,
        .data.assign_expr = (NAssignExpr) {
            .op = op,
            .ident = new_node(&left),
            .value = new_node(&value),
        },
        .left_pos = left.left_pos,
        .right_pos = value.right_pos,
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
        parser_advance(parser); // '<'|'>'|'=='|'!='|'<='|'>='

        Node right = parse_bin_add_expr(parser);
        if (parser->error) return (Node) { 0 };

        left = (Node) {
            .type = NT_BIN_EXPR,
            .data.bin_expr = (NBinExpr) {
                .op = token->type,
                .left = new_node(&left),
                .right = new_node(&right),
            },
            .left_pos = left.left_pos,
            .right_pos = right.right_pos,
        };
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

        if (
            token->type != TT_PLUS
            && token->type != TT_MINUS
        ) {
            break;
        }
        parser_advance(parser); // '+'|'-'

        Node right = parse_bin_mul_expr(parser);
        if (parser->error) return (Node) { 0 };

        left = (Node) {
            .type = NT_BIN_EXPR,
            .data.bin_expr = (NBinExpr) {
                .op = token->type,
                .left = new_node(&left),
                .right = new_node(&right),
            },
            .left_pos = left.left_pos,
            .right_pos = right.right_pos,
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

        if (
            token->type != TT_ASTERISK
            && token->type != TT_SLASH
            && token->type != TT_MODULO
        ) {
            break;
        }
        parser_advance(parser); // '*'|'/'|'%'

        Node right = parse_unary_expr(parser);
        if (parser->error) return (Node) { 0 };

        left = (Node) {
            .type = NT_BIN_EXPR,
            .data.bin_expr = (NBinExpr) {
                .op = token->type,
                .left = new_node(&left),
                .right = new_node(&right),
            },
            .left_pos = left.left_pos,
            .right_pos = right.right_pos,
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
    parser_advance(parser); // '-'|'*'|'!'|'&'

    Node expr = parse_update_expr(parser);
    if (parser->error) return (Node) { 0 };

    expr = (Node) {
        .type = NT_UN_EXPR,
        .data.un_expr = (NUnExpr) {
            .op = token->type,
            .expr = new_node(&expr),
        },
        .left_pos = token->left_pos,
        .right_pos = expr.right_pos,
    };

    return expr;
}

/***
 * ++(expr)
 * --(expr)
 * (expr)++
 * (expr)--
 */
Node parse_update_expr(Parser* parser) {
    const Token* starting_token = parser_at(parser);

    Node expr = { 0 };

    if (starting_token->type != TT_INCREMENT && starting_token->type != TT_DECREMENT) {
        expr = parse_call_expr(parser);
        if (parser->error) return (Node) { 0 };

        // Ignore that semicolon
        if (expr.type == NT_NONE) {
            return expr;
        }

        /* right-hand side operator */
        if (parser_at(parser)->type != TT_INCREMENT && parser_at(parser)->type != TT_DECREMENT) {
            return expr;
        }
        const Token* op = parser_advance(parser); // '++'|'--'

        expr = (Node) {
            .type = NT_UPDATE_EXPR,
            .data.update_expr = (NUpdateExpr) {
                .prefixed = false,
                .op = op->type,
                .expr = new_node(&expr),
            },
            .left_pos = expr.left_pos,
            .right_pos = op->right_pos,
        };

        return expr;
    } else {
        /* left-hand side operator */
        const Token* op = parser_advance(parser); // '++'|'--'

        expr = parse_call_expr(parser);
        if (parser->error) return (Node) { 0 };

        expr = (Node) {
            .type = NT_UPDATE_EXPR,
            .data.update_expr = (NUpdateExpr) {
                .prefixed = true,
                .op = op->type,
                .expr = new_node(&expr),
            },
            .left_pos = op->left_pos,
            .right_pos = expr.right_pos,
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
        if (parser_at(parser)->type != TT_LPAREN) {
            break;
        }

        NodeArr args = parse_args(parser);
        if (parser->error) return (Node) { 0 };

        member = (Node) {
            .type = NT_CALL_EXPR,
            .data.call_expr = (NCallExpr) {
                .member = new_node(&member),
                .args = args,
            },
            .left_pos = member.left_pos,
            .right_pos = args.right_pos,
        };
    }

    return member;
}

/***
 * (member).(member);
 * (member).(member).(...);
 */
Node parse_member_expr(Parser* parser) {
    Node object = parse_cast_expr(parser);
    if (parser->error) return (Node) { 0 };

    // Unwanted tokens non-correlative to the syntax
    if (object.type == NT_NONE) {
        return object;
    }

    while (true) {
        if (parser_at(parser)->type != TT_DOT) {
            break;
        }
        parser_advance(parser); // '.'

        Node property = parse_cast_expr(parser);
        if (parser->error) return (Node) { 0 };

        if (property.type != NT_IDENT_LIT) {
            parser_set_error(parser, "Expected identifier as member property.", property.left_pos);
            return (Node) { 0 };
        }

        object = (Node) {
            .type = NT_MEMBER_EXPR,
            .data.member_expr = (NMemberExpr) {
                .object = new_node(&object),
                .property = new_node(&property),
            },
            .left_pos = object.left_pos,
            .right_pos = property.right_pos,
        };
    }

    return object;
}

/***
 * (<type>) <expr>
 */
Node parse_cast_expr(Parser* parser) {
    /***
     * Parsing cast expression is one of the most seemingly-problematic
     * nodes I've ever encountered. Since it's easy to confuse its pattern
     * with something else's pattern you can create a mismatch and execute
     * the wrong node parsing. An easy solution would be to discard pattern
     * `(<type>) <expr>` and replace it with something like `cast(<type>) <expr>`
     * (less ambiguity, but can be confused with call expr if `cast` is not handled
     * as a keyword)
     * or `<expr> as <type>`, or even just use something like `<expr>.cast(<type>)`
     * and call it a day. But I don't really like those, except for the last one.
     */
    const Token* starting_token = parser_at(parser);

    if (starting_token->type != TT_LPAREN) {
        return parse_literal_term(parser);
    }
    parser_advance(parser); // '('

    Node type = parse_expr(parser);
    if (parser->error) return (Node) { 0 };

    parser_advance_expect(parser, TT_RPAREN, "Expected ')'");
    if (parser->error) return (Node) { 0 };

    /***
     * It's not our cast expression, because the cast expression
     * type is either a member expression, an array type expression or
     * an identifier literal.
     * Also fun fact: This expression function probably discards
     * literal term expressions surrounded by parentheses now, since
     * its pattern (<expr>) matches with literal term expression's (<expr>)
     * pattern.
     */
    if (type.type != NT_MEMBER_EXPR && type.type != NT_ARRAY_TYPE && type.type != NT_IDENT_LIT) {
        return type;
    }

    Node expr = parse_expr(parser);
    if (parser->error) return (Node) { 0 };

    /***
     * Semicolon. Not our cast expression then.
     */
    if (expr.type == NT_NONE) {
        return type;
    }

    Node result = (Node) {
        .type = NT_CAST_EXPR,
        .data.cast_expr = (NCastExpr) {
            .type = new_node(&type),
            .expr = new_node(&expr),
        },
        .left_pos = starting_token->left_pos,
        .right_pos = expr.right_pos,
    };

    return result;
}



/***
 * { }
 * { (ident): (type) }
 * { (ident): (type), (...) }
 */
NodeArr parse_fields(Parser* parser) {
    const Token* starting_token = parser_advance_expect(parser, TT_LBRACE, "Field list expected '{'"); // '{'
    const Token* ending_token = NULL;

    if (parser->error) return (NodeArr) { 0 }; // We have not allocated anything yet, returning instantly

    NodeArr node_array = create_node_arr(64);

    while (true) {
        TokenType token_type = parser_at(parser)->type; // ','|'}'

        if (token_type == TT_RBRACE) {
            ending_token = parser_advance(parser);
            break;
        } else if (token_type == TT_COMMA) {
            parser_advance(parser);

            /* Comma before brace: { ..., ..., } */
            if (parser_at(parser)->type == TT_RBRACE) {
                ending_token = parser_advance(parser);
                break;
            }
        }

        Node ident = parse_expr(parser);
        if (parser->error) goto error;

        parser_advance_expect(parser, TT_COLON, "Expected ':'"); // ':'
        if (parser->error) goto error;

        Node type = parse_type(parser);
        if (parser->error) goto error;

        Node field_node = (Node) {
            .type = NT_FIELD,
            .data.field = (NField) {
                .ident = new_node(&ident),
                .type = new_node(&type),
            },
            .left_pos = ident.left_pos,
            .right_pos = type.right_pos,
        };

        add_to_node_arr(&node_array, field_node);

        token_type = parser_at(parser)->type; // ','|'}'
        if (token_type != TT_RBRACE && token_type != TT_COMMA) {
            parser_set_error(parser, "Expected ',' or '}'", parser_at(parser)->left_pos);
            goto error;
        }
    }

    node_array.left_pos = starting_token->left_pos;
    node_array.right_pos = ending_token->right_pos;

    return node_array;

    error:
    free_node_arr(&node_array);
    return (NodeArr) { 0 };
}

/***
 * ((ident): (type))
 * ((ident): (type), (...))
 */
NodeArr parse_parameters(Parser* parser) {
    const Token* starting_token = parser_advance_expect(parser, TT_LPAREN, "Parameter list expected '('"); // '('
    const Token* ending_token = NULL;
    if (parser->error) return (NodeArr) { 0 }; // We have not allocated anything yet, returning instantly

    NodeArr node_array = create_node_arr(64);

    while (true) {
        TokenType token_type = parser_at(parser)->type;

        if (token_type == TT_RPAREN) {
            ending_token = parser_advance(parser); // ')'
            break;
        }

        Node ident = parse_expr(parser);
        if (parser->error) goto error;

        parser_advance_expect(parser, TT_COLON, "Expected ':'"); // ':'
        if (parser->error) goto error;

        Node type = parse_type(parser);
        if (parser->error) goto error;

        Node param_node = (Node) {
            .type = NT_PARAMETER,
            .data.parameter = (NParameter) {
                .ident = new_node(&ident),
                .type = new_node(&type),
                .defval = NULL,
            },
            .left_pos = ident.left_pos,
            .right_pos = type.right_pos,
        };

        add_to_node_arr(&node_array, param_node);

        const Token* token = parser_advance(parser);
        token_type = token->type; // ','|')'

        if (token_type == TT_RPAREN) {
            ending_token = token;
            break;
        } else if (token_type != TT_COMMA) {
            parser_set_error(parser, "Expected ','", token->left_pos);
            goto error;
        }
    }

    node_array.left_pos = starting_token->left_pos;
    node_array.right_pos = ending_token->right_pos;

    return node_array;

    error:
    free_node_arr(&node_array);
    return (NodeArr) { 0 };
}

/***
 * { (ident) }
 * { (ident), (...) }
 * { (ident) = (expr) }
 * { (ident) = (expr), (...) }
 */
NodeArr parse_enum_entries(Parser* parser) {
    const Token* starting_token = parser_advance_expect(parser, TT_LBRACE, "Enum entries list expected '{'"); // '{'
    const Token* ending_token = NULL;
    if (parser->error) return (NodeArr) { 0 }; // We have not allocated anything yet, returning instantly

    NodeArr node_array = create_node_arr(64);

    while (true) {
        if (parser_at(parser)->type == TT_RBRACE) {
            ending_token = parser_advance(parser);
            break;
        } else if (parser_at(parser)->type == TT_COMMA) {
            parser_advance(parser);
        }

        /* Comma before brace: { ..., ..., } */
        if (parser_at(parser)->type == TT_RBRACE) {
            ending_token = parser_advance(parser);
            break;
        }

        Node ident = parse_literal_term(parser);
        if (parser->error) goto error;

        if (ident.type != NT_IDENT_LIT) {
            parser_set_error(parser, "Expected identifier as Enumeration Entry", ident.left_pos);
            goto error;
        }

        Node value = (Node) { 0 };

        if (parser_at(parser)->type == TT_EQUAL) {
            parser_advance(parser); // '='

            value = parse_expr(parser);
            if (parser->error) goto error;
        }

        Node entry_node = (Node) {
            .type = NT_ENUM_ENTRY,
            .data.enum_entry = (NEnumEntry) {
                .ident = new_node(&ident),
                .value = new_node(&value),
            },
            .left_pos = ident.left_pos,
            .right_pos = value.type == NT_NONE ? ident.right_pos : value.right_pos,
        };

        add_to_node_arr(&node_array, entry_node);

        const Token* token = parser_at(parser);

        if (token->type != TT_RBRACE && token->type != TT_COMMA) {
            parser_set_error(parser, "Expected ',' or '}'", token->left_pos);
            goto error;
        }
    }

    node_array.left_pos = starting_token->left_pos;
    node_array.right_pos = ending_token->right_pos;

    return node_array;

    error:
    free_node_arr(&node_array);
    return (NodeArr) { 0 };
}

/***
 * ((expr))
 * ((expr), (expr), (...))
 */
NodeArr parse_args(Parser* parser) {
    const Token* starting_token = parser_advance_expect(parser, TT_LPAREN, "Parameter list expected '('"); // '('
    const Token* ending_token = NULL;
    if (parser->error) return (NodeArr) { 0 }; // We have not allocated anything yet, returning instantly

    NodeArr node_array = create_node_arr(64);

    while (true) {
        const Token* token = parser_at(parser);
        TokenType token_type = token->type;

        if (token_type == TT_RPAREN) {
            ending_token = parser_advance(parser);
            break;
        } else if (token_type == TT_COMMA) {
            parser_advance(parser);
        }

        Node expr = parse_expr(parser);
        if (parser->error) goto error;

        Node arg_node = (Node) {
            .type = NT_ARGUMENT,
            .data.argument = (NArgument) {
                .expr = new_node(&expr),
                .ident = NULL,
            },
            .left_pos = expr.left_pos,
            .right_pos = expr.right_pos,
        };

        add_to_node_arr(&node_array, arg_node);

        token = parser_at(parser);
        token_type = token->type; // ','|')'

        if (token_type != TT_RPAREN && token_type != TT_COMMA) {
            parser_set_error(parser, "Expected ',' or ')'", token->left_pos);
            goto error;
        }
    }

    node_array.left_pos = starting_token->left_pos;
    node_array.right_pos = ending_token->right_pos;

    return node_array;

    error:
    free_node_arr(&node_array);
    return (NodeArr) { 0 };
}

/***
 * { (...) }
 */
Node parse_block(Parser* parser) {
    const Token* starting_token = parser_advance(parser); // '{'
    const Token* ending_token = NULL;

    NodeArr node_array = create_node_arr(64);

    while (parser->cursor < parser->tokens.count) {
        if (parser_at(parser)->type == TT_RBRACE) {
            ending_token = parser_advance(parser); // '}'
            break;
        }

        Node node = parse_stat(parser);
        if (parser->error) goto error;

        /* Skip ignored nodes */
        if (node.type == NT_NONE) {
            continue;
        }

        // Setting node_array's position range
        if (node_array.count == 0) {
            node_array.left_pos = node.left_pos;
        } else {
            node_array.right_pos = node.right_pos;
        }

        add_to_node_arr(&node_array, node);
    }

    Node result = (Node) {
        .type = NT_BLOCK,
        .data.block = (NBlock) {
            .nodes = node_array,
        },
        .left_pos = starting_token->left_pos,
        .right_pos = ending_token->right_pos,
    };

    return result;

    error:
    free_node_arr(&node_array);
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
    const Token* starting_token = NULL;

    Node number = (Node) { 0 };
    Node type = (Node) { 0 };

    // Poorly structured I think
    while (true) {
        if (parser_at(parser)->type == TT_LBRACKET) {
            starting_token = parser_advance(parser); // '['
        }

        // ArrayType without size
        if (parser_at(parser)->type == TT_RBRACKET) {
            parser_advance(parser); // ']'

            // [](expr)
            type = parse_type(parser);
            if (parser->error) return (Node) { 0 };

            break;
        }

        // [(expr)]
        number = parse_expr(parser);
        if (parser->error) return (Node) { 0 };

        // Array type with size
        if (parser_at(parser)->type == TT_RBRACKET) {
            parser_advance(parser); // ']'

            // [](expr)
            type = parse_type(parser);
            if (parser->error) return (Node) { 0 };
        }

        break;
    }

    Node result = (Node) {
        .type = NT_ARRAY_TYPE,
        .data.array_type = (NArrayType) {
            .number = new_node(&number),
            .type = new_node(&type),
        },
        .left_pos = starting_token->left_pos,
        .right_pos = type.right_pos,
    };

    return result;
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

NodeArr parse_switch_block(Parser* parser) {
    const Token* starting_token = parser_advance(parser); // '{'
    const Token* ending_token = NULL;

    NodeArr node_array = create_node_arr(64);

    bool has_default = false;

    while (true) {
        const Token* token = parser_at(parser);

        if (token->type == TT_RBRACE) {
            ending_token = parser_advance(parser); // '}'
            break;
        } else if (token->type == TT_CASE) {
            const Token* case_token = parser_advance(parser); // 'case'

            Node expr = parse_expr(parser);
            if (parser->error) goto error;

            parser_advance_expect(parser, TT_COLON, "Expected ':'");
            if (parser->error) goto error;

            Node block = parse_block(parser);
            if (parser->error) goto error;

            Node switch_case = (Node) {
                .type = NT_SWITCH_CASE,
                .data.switch_case = (NSwitchCase) {
                    .condition = new_node(&expr),
                    .body = new_node(&block),
                },
                .left_pos = case_token->left_pos,
                .right_pos = block.right_pos,
            };

            add_to_node_arr(&node_array, switch_case);
        } else if (token->type == TT_DEFAULT) {
            const Token* default_token = parser_advance(parser); // 'default'

            if (has_default) {
                parser_set_error(parser, "Multiple default cases in one switch statement", default_token->left_pos);
                goto error;
            }
            has_default = true;

            parser_advance_expect(parser, TT_COLON, "Expected ':'");
            if (parser->error) goto error;

            Node block = parse_block(parser);
            if (parser->error) goto error;

            Node switch_case = (Node) {
                .type = NT_SWITCH_CASE,
                .data.switch_case = (NSwitchCase) {
                    .condition = NULL,
                    .body = new_node(&block),
                },
                .left_pos = default_token->left_pos,
                .right_pos = block.right_pos,
            };

            add_to_node_arr(&node_array, switch_case);
        }
    }

    node_array.left_pos = starting_token->left_pos;
    node_array.right_pos = ending_token->right_pos;

    return node_array;

    error:
    free_node_arr(&node_array);
    return (NodeArr) { 0 };
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
        const Token* break_token = parser_advance(parser); // 'break'

        return (Node) {
            .type = NT_BREAK_STAT,
            .data = {},
            .left_pos = break_token->left_pos,
            .right_pos = break_token->right_pos
        };
    } else if (token->type == TT_CONTINUE) {
        const Token* continue_token = parser_advance(parser); // 'continue'

        return (Node) {
            .type = NT_CONTINUE_STAT,
            .data = {},
            .left_pos = continue_token->left_pos,
            .right_pos = continue_token->right_pos
        };
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
    const Token* return_token = parser_advance(parser); // `return`

    // If the next token is a semicolon, the return value is NONE
    Node value = parse_expr(parser);
    if (parser->error) return (Node) { 0 };

    Node result = (Node) {
        .type = NT_RETURN_STAT,
        .data.ret_stat = (NRetStat) {
            .expr = value.type == NT_NONE ? NULL : new_node(&value),
        },
        .left_pos = return_token->left_pos,
        .right_pos = value.type == NT_NONE ? return_token->right_pos : value.right_pos,
    };

    return result;
}

/***
 * var|const (indent): (type);
 * var|const (indent): (type) = (value);
 */
Node parse_var_stat(Parser* parser) {
    const Token* var_token = parser_advance(parser); // 'var'|'const'
    bool is_const = var_token->type == TT_CONST;

    Node ident = { 0 };
    Node type = { 0 };
    Node value = { 0 };

    ident = parse_literal_term(parser);
    if (parser->error) return (Node) { 0 };

    if (ident.type != NT_IDENT_LIT) {
        parser_set_error(parser, "Expected identifier for variable statement name", ident.left_pos);
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

    Node result = (Node) {
        .type = NT_VAR_STAT,
        .data.var_stat = (NVarStat) {
            .constant = is_const,
            .ident = new_node(&ident),
            .type = new_node(&type),
            .value = value.type == NT_NONE ? NULL : new_node(&value),
        },
        .left_pos = var_token->left_pos,
        .right_pos = value.type == NT_NONE ? type.right_pos : value.right_pos,
    };

    return result;
}

/***
 * enum (ident)
 * enum (ident) { (enum entries) }
 */
Node parse_enum_stat(Parser* parser) {
    const Token* starting_token = parser_advance(parser); // 'enum'

    // Avoid conflict with call expr by using term instead of expr
    Node ident = parse_literal_term(parser);
    if (parser->error) return (Node) { 0 };

    if (ident.type != NT_IDENT_LIT) {
        parser_set_error(parser, "Expected identifier as struct name", ident.left_pos);
        return (Node) { 0 };
    }

    NodeArr entries = { 0 };

    if (parser_at(parser)->type == TT_LBRACE) {
        entries = parse_enum_entries(parser);
        if (parser->error) return (Node) { 0 };
    }

    Node enum_node = (Node) {
        .type = NT_ENUM_STAT,
        .data.enum_stat = (NEnumStat) {
            .ident = new_node(&ident),
            .entries = entries,
        },
        .left_pos = starting_token->left_pos,
        // NOTE: The right_pos point can be wrong with `enum (ident) { (empty) }`
        //       compared to `enum (ident)` or `enum (ident) { (not empty) }`
        .right_pos = entries.count == 0 ? ident.right_pos : entries.right_pos,
    };

    return enum_node;
}

/***
 * struct (ident);
 * struct (ident) (fields)
 */
Node parse_struct_stat(Parser* parser) {
    const Token* starting_token = parser_advance(parser); // `struct`

    // Avoid conflict with call expr by using term instead of expr
    Node ident = parse_literal_term(parser);
    if (parser->error) return (Node) { 0 };

    if (ident.type != NT_IDENT_LIT) {
        parser_set_error(parser, "Expected identifier as struct name", ident.left_pos);
        return (Node) { 0 };
    }

    NodeArr fields = { 0 };

    if (parser_at(parser)->type == TT_LBRACE) {
        fields = parse_fields(parser);
        if (parser->error) return (Node) { 0 };
    }

    Node result = (Node) {
        .type = NT_STRUCT_STAT,
        .data.struct_stat = (NStructStat) {
            .ident = new_node(&ident),
            .fields = fields,
        },
        .left_pos = starting_token->left_pos,
        // NOTE: The right_pos point can be wrong with `struct (ident) { (empty) }`
        //       compared to `struct (ident)` or `struct (ident) { (not empty) }`
        .right_pos = fields.count == 0 ? ident.right_pos : fields.right_pos,
    };

    return result;
}

/***
 * fn (ident) (params): (type) (block)
 */
Node parse_fn_stat(Parser* parser) {
    const Token* fn_token = parser_advance(parser); // 'fn'

    // Avoid conflict with call expr by using term instead of expr
    Node ident = parse_literal_term(parser);
    if (parser->error) return (Node) { 0 };

    if (ident.type != NT_IDENT_LIT) {
        parser_set_error(parser, "Expected identifier as function name", ident.left_pos);
        return (Node) { 0 };
    }

    NodeArr params = parse_parameters(parser);
    if (parser->error) return (Node) { 0 };

    parser_advance_expect(parser, TT_COLON, "Expected ':'"); // ':'
    if (parser->error) return (Node) { 0 };

    Node type = parse_type(parser);
    if (parser->error) return (Node) { 0 };

    parser_at_expect(parser, TT_LBRACE, "Expected '{'"); // '{'
    if (parser->error) return (Node) { 0 };

    Node block = parse_block(parser);
    if (parser->error) return (Node) { 0 };

    Node result = (Node) {
        .type = NT_FUNC_STAT,
        .data.func_stat = (NFuncStat) {
            .ident = new_node(&ident),
            .params = params,
            .type = new_node(&type),
            .body = new_node(&block),
        },
        .left_pos = fn_token->left_pos,
        .right_pos = block.right_pos,
    };

    return result;
}

/***
 * if (expr) (block)
 */
Node parse_if_stat(Parser* parser) {
    const Token* if_token = parser_advance(parser); // `if`

    parser_advance_expect(parser, TT_LPAREN, "The expression in if statement must be closed in parentheses ('(')"); // `(`
    if (parser->error) return (Node) { 0 };

    Node expr = parse_expr(parser);
    if (parser->error) return (Node) { 0 };

    parser_advance_expect(parser, TT_RPAREN, "The expression in if statement must be closed in parentheses (')')"); // `)`
    if (parser->error) return (Node) { 0 };

    parser_at_expect(parser, TT_LBRACE, "Expected '{'"); // `{`
    if (parser->error) return (Node) { 0 };

    /* ---------------- */
    Node body = (Node) { 0 };

    if (parser_at(parser)->type == TT_LBRACE) {
        body = parse_block(parser);
    } else {
        body = parse_expr(parser);
    }

    if (parser->error) return (Node) { 0 };
    /* ---------------- */

    /* ---------------- */
    Node alternate = (Node) { 0 };

    if (parser_at(parser)->type == TT_ELSE) {
        alternate = parse_if_else(parser);
        if (parser->error) return (Node) { 0 };
    }
    /* ---------------- */

    Node result = (Node) {
        .type = NT_IF_STAT,
        .data.if_stat = (NIfStat) {
            .condition = new_node(&expr),
            .body = new_node(&body),
            .alternate = alternate.type == NT_NONE ? NULL : new_node(&alternate),
        },
        .left_pos = if_token->left_pos,
        .right_pos = alternate.type == NT_NONE ? body.right_pos : alternate.right_pos,
    };

    return result;
}

/***
 * while (expr) (block)
 */
Node parse_while_stat(Parser* parser) {
    const Token* while_token = parser_advance(parser); // `while`

    parser_advance_expect(parser, TT_LPAREN, "The expression in while statement must be closed in parentheses ('(')"); // `(`
    if (parser->error) return (Node) { 0 };

    Node expr = parse_expr(parser);
    if (parser->error) return (Node) { 0 };

    parser_advance_expect(parser, TT_RPAREN, "The expression in while statement must be closed in parentheses (')')"); // `)`
    if (parser->error) return (Node) { 0 };

    /* ---------------- */
    Node body = (Node) { 0 };
    const Token* token = parser_at(parser);

    if (token->type == TT_LBRACE) {
        body = parse_block(parser);
    } else {
        body = parse_expr(parser);
    }

    if (parser->error) return (Node) { 0 };
    /* ---------------- */

    Node result = (Node) {
        .type = NT_WHILE_STAT,
        .data.while_stat = (NWhileStat) {
            .condition = new_node(&expr),
            .body = body.type == NT_NONE ? NULL : new_node(&body),
        },
        .left_pos = while_token->left_pos,
        .right_pos = body.right_pos,
    };

    return result;
}

/***
 * switch (expr) {}
 * switch (expr) { case (expr): (block), (...) }
 * switch (expr) { default: (block) }
 */
Node parse_switch_stat(Parser* parser) {
    const Token* switch_token = parser_advance(parser); // 'switch'

    parser_advance_expect(parser, TT_LPAREN, "Expected '(' in switch statement");
    if (parser->error) return (Node) { 0 };

    Node lookup = parse_expr(parser);
    if (parser->error) return (Node) { 0 };

    parser_advance_expect(parser, TT_RPAREN, "Expected ')' in switch statement");
    if (parser->error) return (Node) { 0 };

    NodeArr cases = parse_switch_block(parser);
    if (parser->error) return (Node) { 0 };

    Node switch_node = (Node) {
        .type = NT_SWITCH_STAT,
        .data.switch_stat = (NSwitchStat) {
            .lookup = new_node(&lookup),
            .cases = cases,
        },
        .left_pos = switch_token->left_pos,
        .right_pos = cases.right_pos,
    };

    return switch_node;
}
