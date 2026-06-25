#ifndef AST_H
#define AST_H

#include <assert.h>
#include <parser.h>
#include <stdint.h>

/***
 * NODE TREE PRINT
 */
typedef void (*PrintNodeTree_Func)(Node, int32_t);
extern const PrintNodeTree_Func PrintNodeTree_Funcs[];

void print_node_tree(Node node, int32_t indent);

// Program
void print_program_node_tree(Node node, int32_t indent);

// Statements
void print_return_stat_tree(Node node, int32_t indent);
void print_break_stat_tree(Node node, int32_t indent);
void print_continue_stat_tree(Node node, int32_t indent);
void print_var_stat_tree(Node node, int32_t indent);
void print_enum_stat_tree(Node node, int32_t indent);
void print_struct_stat_tree(Node node, int32_t indent);
void print_fn_stat_tree(Node node, int32_t indent);
void print_if_stat_tree(Node node, int32_t indent);
void print_while_stat_tree(Node node, int32_t indent);
void print_switch_stat_tree(Node node, int32_t indent);

// Standalone Expressions
void print_block_node_tree(Node node, int32_t indent);
void print_param_list_node_tree(Node node, int32_t indent);
void print_param_node_tree(Node node, int32_t indent);
void print_enum_entry_list_node_tree(Node node, int32_t indent);
void print_enum_entry_node_tree(Node node, int32_t indent);
void print_field_list_node_tree(Node node, int32_t indent);
void print_field_node_tree(Node node, int32_t indent);
void print_arg_list_node_tree(Node node, int32_t indent);
void print_arg_node_tree(Node node, int32_t indent);
void print_array_type_tree(Node node, int32_t indent);
void print_switch_entry_list_node_tree(Node node, int32_t indent);
void print_switch_entry_node_tree(Node node, int32_t indent);

// Expressions
void print_array_expr_tree(Node node, int32_t indent);
void print_assign_expr_tree(Node node, int32_t indent);
void print_bin_expr_tree(Node node, int32_t indent);
void print_unary_expr_tree(Node node, int32_t indent);
void print_update_expr_tree(Node node, int32_t indent);
void print_call_expr_tree(Node node, int32_t indent);
void print_member_expr_tree(Node node, int32_t indent);

// Literals
void print_integer_lit_tree(Node node, int32_t indent);
void print_float_lit_tree(Node node, int32_t indent);
void print_string_lit_tree(Node node, int32_t indent);
void print_ident_lit_tree(Node node, int32_t indent);


/***
 * LITERALS
 */
Node parse_literal_term(Parser* parser);

/***
 * EXPRESSIONS
 */
Node parse_expr(Parser* parser);

Node parse_array_expr(Parser* parser);

Node parse_assign_expr(Parser* parser);

Node parse_bin_comp_expr(Parser* parser);

Node parse_bin_add_expr(Parser* parser);

Node parse_bin_mul_expr(Parser* parser);

// TODO: Combine UNARY and UPDATE expressions if needed
Node parse_unary_expr(Parser* parser);

Node parse_update_expr(Parser* parser);

Node parse_call_expr(Parser* parser);

Node parse_member_expr(Parser* parser);

/***
 * STANDALONE EXPRESSIONS
 * : Expressions that are either kinda also statements
 * or just used by other nodes
 */
Node parse_fields(Parser* parser);

Node parse_parameters(Parser* parser);

Node parse_enum_entries(Parser* parser);

Node parse_args(Parser* parser);

Node parse_block(Parser* parser);

Node parse_type(Parser* parser);

Node parse_array_type(Parser* parser);

Node parse_if_else(Parser* parser);

Node parse_switch_block(Parser* parser);

/***
 * STATEMENTS
 */
Node parse_stat(Parser* parser);

Node parse_var_stat(Parser* parser);

Node parse_enum_stat(Parser* parser);

Node parse_struct_stat(Parser* parser);

Node parse_fn_stat(Parser* parser);

Node parse_if_stat(Parser* parser);

Node parse_while_stat(Parser* parser);

Node parse_switch_stat(Parser* parser);

Node parse_return_stat(Parser* parser);

/***
 * Parsing
 */
Node parse_tokens(Parser* parser);

#endif
