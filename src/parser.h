#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include <lexer.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <value.h>

#define INITIAL_NODE_POOL_SIZE (1024 * 16) // 16 KB
#define PARSER_ERROR_SIZE 256

typedef enum NodeType {
    NT_NONE,

    NT_PROGRAM,

    NT_RETURN_STAT,
    NT_BREAK_STAT,
    NT_CONTINUE_STAT,
    NT_DEFER_STAT,
    NT_VAR_STAT,
    NT_ENUM_STAT,
    NT_STRUCT_STAT,
    NT_FUNC_STAT,
    NT_IF_STAT,
    NT_WHILE_STAT,
    NT_SWITCH_STAT,

    NT_BIN_EXPR,
    NT_UN_EXPR,
    NT_UPDATE_EXPR,
    NT_TERN_EXPR,
    NT_ASSIGN_EXPR,
    NT_CALL_EXPR,
    NT_MEMBER_EXPR,
    NT_CAST_EXPR,

    NT_BLOCK,
    NT_PARAMETER,
    NT_ENUM_ENTRY,
    NT_ARGUMENT,
    NT_FIELD,
    NT_ARRAY_TYPE,
    NT_SWITCH_CASE,

    NT_INTEGER_LIT,
    NT_FLOAT_LIT,
    NT_STRING_LIT,
    NT_IDENT_LIT,
    NT_ARRAY_LIT,
    NT_COMPOUND_LIT,
} NodeType;

extern const char* NodeTypeNames[];

typedef struct Node Node;

typedef struct NodeArr {
    Node*  nodes;
    size_t count;
    size_t capacity;

    TokenPosition left_pos;
    TokenPosition right_pos;
} NodeArr;

NodeArr create_node_arr(size_t size);
void free_node_arr(NodeArr* narr);
void add_to_node_arr(NodeArr* narr, Node node);

/***
 * NODE TYPES
 */

/* Program */
typedef struct NProgram {
    NodeArr nodes;
} NProgram;

/* LITERALS */
typedef struct NIntLit {
    char*  value;
    size_t size;
} NIntLit;

typedef struct NFloatLit {
    char*  value;
    size_t size;
} NFloatLit;

typedef struct NStringLit {
    char*  value;
    size_t size;
} NStringLit;

typedef struct NIdentLit {
    char*  value;
    size_t size;
} NIdentLit;

typedef struct NArrayLit {
    NodeArr values;
} NArrayLit;

typedef struct NCompoundLit {
    NodeArr values;
} NCompoundLit;


/* STATEMENTS */
typedef struct NRetStat {
    Node* expr;
} NRetStat;

typedef struct NDeferStat {
    Node* expr;
} NDeferStat;

typedef struct NVarStat {
    bool  constant;
    Node* ident;
    Node* type;
    Node* value;
} NVarStat;

typedef struct NEnumStat {
    Node*   ident;
    NodeArr entries;
} NEnumStat;

typedef struct NStructStat {
    Node*   ident;
    NodeArr fields;
} NStructStat;

typedef struct NFuncStat {
    Node*   ident;
    NodeArr params;
    Node*   type;
    Node*   body;
} NFuncStat;

typedef struct NIfStat {
    Node* condition;
    Node* body;
    Node* alternate;
} NIfStat;

typedef struct NWhileStat {
    Node* condition;
    Node* body;
} NWhileStat;

typedef struct NSwitchStat {
    Node*   lookup;
    NodeArr cases;
} NSwitchStat;


/* STANDALONE NODES */
typedef struct NBlock {
    NodeArr nodes;
} NBlock;

typedef struct NParameter {
    Node* ident;
    Node* type;
    Node* defval; // default value
} NParameter;

typedef struct NField {
    Node* ident;
    Node* type;
} NField;

typedef struct NArgument {
    Node* ident; // (expr, expr, ident: expr)
    Node* expr;
} NArgument;

typedef struct NEnumEntry {
    Node* ident;
    Node* value;
} NEnumEntry;

typedef struct NSwitchCase {
    Node* condition;
    Node* body;
} NSwitchCase;

typedef struct NArrayType {
    Node* number; // [number]
    Node* type;
} NArrayType;


/* EXPRESSIONS */
typedef struct NAssignExpr {
    TokenType op;
    Node*     ident;
    Node*     value;
} NAssignExpr;

typedef struct NBinExpr {
    TokenType op;
    Node*     left;
    Node*     right;
} NBinExpr;

typedef struct NUnExpr {
    TokenType op;
    Node*     expr;
} NUnExpr;

typedef struct NUpdateExpr {
    bool      prefixed;
    TokenType op;
    Node*     expr;
} NUpdateExpr;

typedef struct NCallExpr {
    Node*   member;
    NodeArr args;
} NCallExpr;

typedef struct NMemberExpr {
    Node* object;
    Node* property;
} NMemberExpr;

typedef struct NCastExpr {
    Node* type;
    Node* expr;
} NCastExpr;

/***
 * NODE
 */
typedef union NodeData {
    NProgram program;

    NIntLit      int_lit;
    NFloatLit    float_lit;
    NStringLit   string_lit;
    NIdentLit    ident_lit;
    NArrayLit    array_lit;
    NCompoundLit compound_lit;

    NRetStat    ret_stat;
    NDeferStat  defer_stat;
    NVarStat    var_stat;
    NEnumStat   enum_stat;
    NStructStat struct_stat;
    NFuncStat   func_stat;
    NIfStat     if_stat;
    NWhileStat  while_stat;
    NSwitchStat switch_stat;

    NBlock      block;
    NParameter  parameter;
    NField      field;
    NArgument   argument;
    NEnumEntry  enum_entry;
    NSwitchCase switch_case;
    NArrayType  array_type;

    NAssignExpr assign_expr;
    NBinExpr    bin_expr;
    NUnExpr     un_expr;
    NUpdateExpr update_expr;
    NCallExpr   call_expr;
    NMemberExpr member_expr;
    NCastExpr   cast_expr;
} NodeData;

typedef struct Node {
    NodeType type;
    NodeData data;

    TokenPosition left_pos;
    TokenPosition right_pos;
} Node;



/***
 * PARSER
 */
typedef struct Parser {
    TokenArray tokens;
    size_t     cursor;

    char errmsg[PARSER_ERROR_SIZE];
    bool error;
    TokenPosition errpos;

    Node** allocations;
    size_t allocation_count;
} Parser;

Parser create_parser(TokenArray token_array);
void free_parser(Parser* parser);
const Token* parser_at(Parser* parser);
const Token* parser_advance(Parser* parser);
const Token* parser_at_expect(Parser* parser, TokenType token_type, const char* errmsg);
const Token* parser_advance_expect(Parser* parser, TokenType token_type, const char* errmsg);
void parser_set_error(Parser* parser, const char* errmsg, TokenPosition errpos);
const char* parser_get_error(Parser* parser);
Node parse_tokens(Parser* parser);

#endif
