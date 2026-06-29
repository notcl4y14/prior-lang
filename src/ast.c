#include "lexer.h"
#include "mem.h"
#include "parser.h"
#include "token.h"
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
    const Token* token = parser_advance(parser);

    if (token->type == TT_INTEGER) {
        Node node = (Node) { .type = NT_INTEGER_LIT, {} };
        node.data.int_lit.value = str_alloc_copy(token->value);
        node.data.int_lit.size = (size_t) strlen(token->value) + 1;

        return node;
    } else if (token->type == TT_FLOAT) {
        Node node = (Node) { .type = NT_FLOAT_LIT, {} };
        node.data.int_lit.value = str_alloc_copy(token->value);
        node.data.int_lit.size = (size_t) strlen(token->value) + 1;

        return node;
    } else if (token->type == TT_STRING) {
        Node node = (Node) { .type = NT_STRING_LIT, {} };
        node.data.int_lit.value = str_alloc_copy(token->value);
        node.data.int_lit.size = (size_t) strlen(token->value) + 1;

        return node;
    } else if (token->type == TT_IDENTIFIER) {
        Node node = (Node) { .type = NT_IDENT_LIT, {} };
        node.data.int_lit.value = str_alloc_copy(token->value);
        node.data.int_lit.size = (size_t) strlen(token->value) + 1;

        return node;
    } else if (token->type == TT_LPAREN) {
        Node expr = parse_expr(parser);

        parser_advance_expect(parser, TT_RPAREN, "Expected ')'");
        if (parser->error) return (Node) { 0 };

        return expr;
    } else if (token->type  == TT_SEMICOLON) {
        /* Ignore semicolons */
        Node result = (Node) { .type = NT_NONE, {} };

        return result;
    }

    parser_set_error(parser, "Unexpected token '%c'", token->left_pos);
    return (Node) { .type = NT_NONE, {} };
}

/***
 * [ (expr) ]
 * [ (expr), (...) ]
 * [ (expr), ]
 * [ (expr), (...), ]
 */
Node parse_array_lit(Parser* parser) {
    parser_advance(parser); // '['

    NodeArr node_array = create_node_arr(64);

    while (true) {
        if (parser_at(parser)->type == TT_RBRACKET) {
            parser_advance(parser); // ']'
            break;
        }


        Node expr = parse_expr(parser);
        if (parser->error) goto error;

        add_to_node_arr(&node_array, expr);


        if (parser_at(parser)->type == TT_COMMA) {
            parser_advance(parser); // ','
            continue;
        } else if (parser_at(parser)->type == TT_RBRACKET) {
            parser_advance(parser); // ']'
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
    };

    return array_node;

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

    Node left = parse_update_expr(parser);
    if (parser->error) return (Node) { 0 };

    left = (Node) {
        .type = NT_UN_EXPR,
        .data.un_expr = (NUnExpr) {
            .op = token->type,
            .expr = new_node(&left),
        },
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

    Node expr = { 0 };

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
        parser_advance(parser); // '++'|'--'

        expr = (Node) {
            .type = NT_UPDATE_EXPR,
            .data.update_expr = (NUpdateExpr) {
                .prefixed = false,
                .op = token->type,
                .expr = new_node(&expr),
            },
        };

        return expr;
    } else {
        /* left-hand side operator */
        parser_advance(parser); // '++'|'--'

        expr = parse_call_expr(parser);
        if (parser->error) return (Node) { 0 };

        expr = (Node) {
            .type = NT_UPDATE_EXPR,
            .data.update_expr = (NUpdateExpr) {
                .prefixed = true,
                .op = token->type,
                .expr = new_node(&expr),
            },
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
        if (parser_at(parser)->type != TT_DOT) {
            break;
        }
        parser_advance(parser); // '.'

        TokenPosition property_pos = parser_at(parser)->left_pos;
        Node property = parse_literal_term(parser);
        if (parser->error) return (Node) { 0 };

        if (property.type != NT_IDENT_LIT) {
            parser_set_error(parser, "Expected identifier as member property.", property_pos);
            return (Node) { 0 };
        }

        object = (Node) {
            .type = NT_MEMBER_EXPR,
            .data.member_expr = (NMemberExpr) {
                .object = new_node(&object),
                .property = new_node(&property),
            },
        };
    }

    return object;
}



/***
 * { }
 * { (ident): (type) }
 * { (ident): (type), (...) }
 */
NodeArr parse_fields(Parser* parser) {
    parser_advance_expect(parser, TT_LBRACE, "Field list expected '{'"); // '{'
    if (parser->error) return (NodeArr) { 0 }; // We have not allocated anything yet, returning instantly

    NodeArr node_array = create_node_arr(64);

    while (true) {
        TokenType token_type = parser_at(parser)->type; // ','|'}'

        if (token_type == TT_RBRACE) {
            parser_advance(parser);
            break;
        } else if (token_type == TT_COMMA) {
            parser_advance(parser);

            /* [ ..., ..., ] */
            if (parser_at(parser)->type == TT_RBRACE) {
                parser_advance(parser);
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
        };

        add_to_node_arr(&node_array, field_node);

        token_type = parser_at(parser)->type; // ','|'}'
        if (token_type != TT_RBRACE && token_type != TT_COMMA) {
            parser_set_error(parser, "Expected ',' or '}'", parser_at(parser)->left_pos);
            goto error;
        }
    }

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
    parser_advance_expect(parser, TT_LPAREN, "Parameter list expected '('"); // '('
    if (parser->error) return (NodeArr) { 0 }; // We have not allocated anything yet, returning instantly

    NodeArr node_array = create_node_arr(64);

    while (true) {
        TokenType token_type = parser_at(parser)->type;

        if (token_type == TT_RPAREN) {
            parser_advance(parser); // ')'
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
        };

        add_to_node_arr(&node_array, param_node);

        const Token* token = parser_advance(parser);
        token_type = token->type; // ','|')'

        if (token_type == TT_RPAREN) {
            break;
        } else if (token_type != TT_COMMA) {
            parser_set_error(parser, "Expected ','", token->left_pos);
            goto error;
        }
    }

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
    parser_advance_expect(parser, TT_LBRACE, "Enum entries list expected '{'"); // '{'
    if (parser->error) return (NodeArr) { 0 }; // We have not allocated anything yet, returning instantly

    NodeArr node_array = create_node_arr(64);

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

        Node entry_node = (Node) {
            .type = NT_ENUM_ENTRY,
            .data.enum_entry = (NEnumEntry) {
                .ident = new_node(&ident),
                .value = new_node(&value),
            },
        };

        add_to_node_arr(&node_array, entry_node);

        token = parser_at(parser);

        if (token->type != TT_RBRACE && token->type != TT_COMMA) {
            parser_set_error(parser, "Expected ',' or '}'", token->left_pos);
            goto error;
        }
    }

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
    parser_advance_expect(parser, TT_LPAREN, "Parameter list expected '('"); // '('
    if (parser->error) return (NodeArr) { 0 }; // We have not allocated anything yet, returning instantly

    NodeArr node_array = create_node_arr(64);

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

        Node arg_node = (Node) {
            .type = NT_ARGUMENT,
            .data.argument = (NArgument) {
                .expr = new_node(&expr),
                .ident = NULL,
            },
        };

        add_to_node_arr(&node_array, arg_node);

        token = parser_at(parser);
        token_type = token->type; // ','|')'

        if (token_type != TT_RPAREN && token_type != TT_COMMA) {
            parser_set_error(parser, "Expected ',' or ')'", token->left_pos);
            goto error;
        }
    }

    return node_array;

    error:
    free_node_arr(&node_array);
    return (NodeArr) { 0 };
}

/***
 * { (...) }
 */
Node parse_block(Parser* parser) {
    parser_advance(parser); // '{'

    NodeArr node_array = create_node_arr(64);

    while (parser->cursor < parser->tokens.count) {
        if (parser_at(parser)->type == TT_RBRACE) {
            parser_advance(parser); // '}'
            break;
        }

        Node node = parse_stat(parser);
        if (parser->error) goto error;

        /* Skip ignored nodes */
        if (node.type == NT_NONE) {
            continue;
        }

        add_to_node_arr(&node_array, node);
    }

    Node result = (Node) {
        .type = NT_BLOCK,
        .data.block = (NBlock) {
            .nodes = node_array,
        },
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
    const Token* token = NULL;

    Node number = (Node) { 0 };
    Node type = (Node) { 0 };

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
            type = parse_type(parser);
            if (parser->error) return (Node) { 0 };

            break;
        }

        // [(expr)]
        number = parse_expr(parser);
        if (parser->error) return (Node) { 0 };

        token = parser_at(parser);

        // Array type with size
        if (token->type == TT_RBRACKET) {
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
    parser_advance(parser); // '{'

    NodeArr node_array = create_node_arr(64);

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

            Node switch_case = (Node) {
                .type = NT_SWITCH_CASE,
                .data.switch_case = (NSwitchCase) {
                    .condition = new_node(&expr),
                    .body = new_node(&block),
                },
            };

            add_to_node_arr(&node_array, switch_case);
        } else if (token->type == TT_DEFAULT) {
            token = parser_advance(parser); // 'default'

            if (has_default) {
                parser_set_error(parser, "Multiple default cases in one switch statement", token->left_pos);
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
            };

            add_to_node_arr(&node_array, switch_case);
        }
    }

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
        parser_advance(parser); // 'break'

        Node node = (Node) { .type = NT_BREAK_STAT, {} };

        return node;
    } else if (token->type == TT_CONTINUE) {
        parser_advance(parser); // 'continue'

        Node node = (Node) { .type = NT_CONTINUE_STAT, {} };

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

    if (value.type == NT_NONE) {
        // Do nothing
    }

    Node result = (Node) {
        .type = NT_RETURN_STAT,
        .data.ret_stat = (NRetStat) {
            .expr = value.type == NT_NONE ? NULL : new_node(&value),
        },
    };

    return result;
}

/***
 * var|const (indent): (type);
 * var|const (indent): (type) = (value);
 */
Node parse_var_stat(Parser* parser) {
    TokenType token_type = parser_advance(parser)->type; // 'var'|'const'
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

    Node result = (Node) {
        .type = NT_VAR_STAT,
        .data.var_stat = (NVarStat) {
            .constant = is_const,
            .ident = new_node(&ident),
            .type = new_node(&type),
            .value = value.type == NT_NONE ? NULL : new_node(&value),
        },
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

    NodeArr entries = { 0 };

    TokenType token_type = parser_at(parser)->type;

    if (token_type == TT_LBRACE) {
        entries = parse_enum_entries(parser);
        if (parser->error) return (Node) { 0 };
    }

    Node enum_node = (Node) {
        .type = NT_ENUM_STAT,
        .data.enum_stat = (NEnumStat) {
            .ident = new_node(&ident),
            .entries = entries,
        },
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

    NodeArr fields = { 0 };

    TokenType token_type = parser_at(parser)->type;

    if (token_type == TT_LBRACE) {
        fields = parse_fields(parser);
        if (parser->error) return (Node) { 0 };
    }

    Node result = (Node) {
        .type = NT_STRUCT_STAT,
        .data.struct_stat = (NStructStat) {
            .ident = new_node(&ident),
            .fields = fields,
        },
    };

    return result;
}

/***
 * fn (ident) (params): (type) (block)
 */
Node parse_fn_stat(Parser* parser) {
    parser_advance(parser); // 'fn'

    TokenPosition token_pos = parser_at(parser)->left_pos;

    // Avoid conflict with call expr
    Node ident = parse_literal_term(parser);
    if (parser->error) return (Node) { 0 };

    if (ident.type != NT_IDENT_LIT) {
        parser_set_error(parser, "Expected identifier as function name", token_pos);
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

    Node result = (Node) {
        .type = NT_IF_STAT,
        .data.if_stat = (NIfStat) {
            .condition = new_node(&expr),
            .body = new_node(&block),
            .alternate = ifelse.type == NT_NONE ? NULL : new_node(&ifelse),
        }
    };

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

    Node result = (Node) {
        .type = NT_WHILE_STAT,
        .data.while_stat = (NWhileStat) {
            .condition = new_node(&expr),
            .body = block.type == NT_NONE ? NULL : new_node(&block),
        },
    };

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

    NodeArr cases = parse_switch_block(parser);
    if (parser->error) return (Node) { 0 };

    Node switch_node = (Node) {
        .type = NT_SWITCH_STAT,
        .data.switch_stat = (NSwitchStat) {
            .lookup = new_node(&lookup),
            .cases = cases,
        },
    };

    return switch_node;
}
