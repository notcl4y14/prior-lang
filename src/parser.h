#ifndef PARSER_H
#define PARSER_H

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
    NT_VAR_STAT,
    NT_ENUM_STAT,
    NT_STRUCT_STAT,
    NT_FUNCTION_STAT,
    NT_IF_STAT,
    NT_WHILE_STAT,
    NT_SWITCH_STAT,

    NT_ARRAY_EXPR,
    NT_BIN_EXPR,
    NT_UNARY_EXPR,
    NT_UPDATE_EXPR,
    NT_TERN_EXPR,
    NT_ASSIGN_EXPR,
    NT_CALL_EXPR,
    NT_MEMBER_EXPR,

    NT_BLOCK_EXPR,
    NT_PARAMLIST,
    NT_PARAM,
    NT_ENUM_ENTRY_LIST,
    NT_ENUM_ENTRY,
    NT_ARGLIST,
    NT_ARG,
    NT_FIELDLIST,
    NT_FIELD,
    NT_ARRAY_TYPE,
    NT_SWITCH_ENTRY_LIST,
    NT_SWITCH_ENTRY,

    NT_INTEGER_LIT,
    NT_FLOAT_LIT,
    NT_STRING_LIT,
    NT_IDENT_LIT,
} NodeType;

extern const char* NodeTypeNames[];

// TODO: Improve this
typedef struct Node {
    NodeType type;
    void* pool_ptr;
} Node;



/***
 * NODE DATA STRUCTURES
 */
typedef struct NodeReturnStatData {
    Node value;
} NodeReturnStatData;

typedef struct NodeVarStatData {
    bool is_const;
    Node ident;
    Node type;
    Node value;

    ValueType return_type;
} NodeVarStatData;

typedef struct NodeEnumData {
    Node ident;
    Node entries;
} NodeEnumData;

typedef struct NodeStructData {
    Node ident;
    Node fields;
} NodeStructData;

typedef struct NodeFunctionData {
    Node ident;
    Node params;
    Node type;
    Node block;
} NodeFunctionData;

typedef struct NodeIfStatData {
    Node expr;
    Node block;
    Node ifelse;
} NodeIfStatData;

typedef struct NodeWhileData {
    Node expr;
    Node block;
} NodeWhileData;

typedef struct NodeSwitchData {
    Node lookup;
    Node block;
} NodeSwitchData;



typedef struct NodeArrayExprData {
    Node*  nodes;
    size_t count;
} NodeArrayExprData;

typedef struct NodeAssignExprData {
    TokenType op;
    Node ident;
    Node value;
} NodeAssignExprData;

typedef struct NodeBinExprData {
    TokenType op;
    Node      left;
    Node      right;

    ValueType return_type;
} NodeBinExprData;

typedef struct NodeUnaryExprData {
    TokenType op;
    Node      expr;

    ValueType return_type;
} NodeUnaryExprData;

typedef struct NodeUpdateExprData {
    TokenType op;
    bool      prefix;
    Node      expr;

    ValueType return_type;
} NodeUpdateExprData;

typedef struct NodeCallExprData {
    Node member;
    Node args;
} NodeCallExprData;

typedef struct NodeMemberExprData {
    Node object;
    Node property;
} NodeMemberExprData;



typedef struct NodeBlockData {
    Node*  nodes;
    size_t count;
} NodeBlockData;

typedef struct NodeParamData {
    Node name;
    Node type;
} NodeParamData;

typedef struct NodeParamListData {
    Node*  nodes;
    size_t count;
} NodeParamListData;

typedef struct NodeFieldData {
    Node name;
    Node type;
} NodeFieldData;

typedef struct NodeFieldListData {
    Node*  nodes;
    size_t count;
} NodeFieldListData;

typedef struct NodeEnumEntryData {
    Node name;
    Node value;
} NodeEnumEntryData;

typedef struct NodeEnumEntryListData {
    Node*  nodes;
    size_t count;
} NodeEnumEntryListData;

typedef struct NodeArgData {
    Node expr;
} NodeArgData;

typedef struct NodeArgListData {
    Node*  nodes;
    size_t count;
} NodeArgListData;

typedef struct NodeArrayTypeData {
    Node size;
    Node type;
} NodeArrayTypeData;

typedef struct NodeSwitchEntryListData {
    Node*  entries;
    size_t count;
} NodeSwitchEntryListData;

typedef struct NodeSwitchEntryData {
    bool is_default;
    Node expr;
    Node block;
} NodeSwitchEntryData;



typedef struct NodeLiteralData {
    char*  value;
    size_t size;
} NodeLiteralData;



/***
 * NODE POOL
 */
typedef struct NodePool {
    void*  data;
    size_t count;
    size_t capacity;
} NodePool;

void init_node_pool(NodePool* npool, size_t size);
void free_node_pool(NodePool* npool);
void* allocate_node_pool(NodePool* npool, size_t size);
void* allocate_and_write_node_pool(NodePool* npool, const void* data, size_t size);



/***
 * PARSER
 */
typedef struct Parser {
    TokenArray* tokens;
    size_t cursor;
    NodePool node_pool;

    char errmsg[PARSER_ERROR_SIZE];
    bool error;
    TokenPosition errpos;
} Parser;

Parser create_parser(TokenArray* token_array);
void free_parser(Parser* parser);
const Token* parser_at(Parser* parser);
const Token* parser_advance(Parser* parser);
const Token* parser_at_expect(Parser* parser, TokenType token_type, const char* errmsg);
const Token* parser_advance_expect(Parser* parser, TokenType token_type, const char* errmsg);
void parser_set_error(Parser* parser, const char* errmsg, TokenPosition errpos);
const char* parser_get_error(Parser* parser);
Node parse_tokens(Parser* parser);

#endif
