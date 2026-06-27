#include "parser.h"
#include <ast.h>
#include <stdio.h>

const PrintNodeTree_Func PrintNodeTree_Funcs[] = {
    [NT_PROGRAM] = print_program_node_tree,

    [NT_RETURN_STAT] = print_return_stat_tree,
    [NT_BREAK_STAT] = print_break_stat_tree,
    [NT_CONTINUE_STAT] = print_continue_stat_tree,
    [NT_VAR_STAT] = print_var_stat_tree,
    [NT_ENUM_STAT] = print_enum_stat_tree,
    [NT_STRUCT_STAT] = print_struct_stat_tree,
    [NT_FUNCTION_STAT] = print_fn_stat_tree,
    [NT_IF_STAT] = print_if_stat_tree,
    [NT_WHILE_STAT] = print_while_stat_tree,
    [NT_SWITCH_STAT] = print_switch_stat_tree,

    [NT_BLOCK_EXPR] = print_block_node_tree,
    [NT_PARAMLIST] = print_param_list_node_tree,
    [NT_PARAM] = print_param_node_tree,
    [NT_ENUM_ENTRY_LIST] = print_enum_entry_list_node_tree,
    [NT_ENUM_ENTRY] = print_enum_entry_node_tree,
    [NT_FIELDLIST] = print_field_list_node_tree,
    [NT_FIELD] = print_field_node_tree,
    [NT_ARGLIST] = print_arg_list_node_tree,
    [NT_ARG] = print_arg_node_tree,
    [NT_ARRAY_TYPE] = print_array_type_tree,
    [NT_SWITCH_ENTRY_LIST] = print_switch_entry_list_node_tree,
    [NT_SWITCH_ENTRY] = print_switch_entry_node_tree,

    [NT_ARRAY_EXPR] = print_array_expr_tree,
    [NT_ASSIGN_EXPR] = print_assign_expr_tree,
    [NT_BIN_EXPR] = print_bin_expr_tree,
    [NT_UNARY_EXPR] = print_unary_expr_tree,
    [NT_UPDATE_EXPR] = print_update_expr_tree,
    [NT_CALL_EXPR] = print_call_expr_tree,
    [NT_MEMBER_EXPR] = print_member_expr_tree,

    [NT_INTEGER_LIT] = print_integer_lit_tree,
    [NT_FLOAT_LIT] = print_float_lit_tree,
    [NT_STRING_LIT] = print_string_lit_tree,
    [NT_IDENT_LIT] = print_ident_lit_tree,
};

void print_indent(int32_t indent) {
    for (int32_t i = 0; i < indent; ++i) {
        printf(" ");
    }
}



void print_node_tree(Node node, int32_t indent) {
    if (node.type < 0 || node.type >= sizeof(PrintNodeTree_Funcs)) {
        printf("WARNING: %s not handled in print_node_tree\n", NodeTypeNames[node.type]);
        return;
    } else if (PrintNodeTree_Funcs[node.type] == NULL) {
        printf("WARNING: %s not handled in print_node_tree\n", NodeTypeNames[node.type]);
        return;
    }

    PrintNodeTree_Funcs[node.type](node, indent);
}



void print_program_node_tree(Node node, int32_t indent) {
    printf("[PROGRAM]:\n");
    NodeBlockData* block_data = (NodeBlockData*) node.pool_ptr;

    for (int32_t i = 0; i < block_data->count; ++i) {
        print_node_tree(block_data->nodes[i], indent + 1);
    }
}




void print_return_stat_tree(Node node, int32_t indent) {
    NodeReturnStatData* varstat = (NodeReturnStatData*) node.pool_ptr;

    print_indent(indent);
    printf("[RETURN_STAT]:\n");

    print_indent(indent + 1); // TODO: remove this, idk why I put this here
    print_node_tree(varstat->value, indent + 1);
}

void print_break_stat_tree(Node node, int32_t indent) {
    print_indent(indent);
    printf("[BREAK_STAT]\n");
}

void print_continue_stat_tree(Node node, int32_t indent) {
    print_indent(indent);
    printf("[CONTINUE_STAT]\n");
}

void print_var_stat_tree(Node node, int32_t indent) {
    NodeVarStatData* varstat = (NodeVarStatData*) node.pool_ptr;

    print_indent(indent);
    printf("[VAR_STAT]:\n");

    print_indent(indent + 1);
    printf("%s\n", varstat->is_const ? "CONST" : "VAR");

    print_node_tree(varstat->ident, indent + 1);
    print_node_tree(varstat->type, indent + 1);

    if (varstat->value.type == NT_NONE) {
        print_indent(indent + 1);
        printf("NO DEFINITION\n");
    } else {
        print_node_tree(varstat->value, indent + 1);
    }
}

void print_enum_stat_tree(Node node, int32_t indent) {
    NodeEnumData* enum_data = (NodeEnumData*) node.pool_ptr;

    print_indent(indent);
    printf("[ENUM_STAT]:\n");

    print_node_tree(enum_data->ident, indent + 1);

    if (enum_data->entries.type == NT_NONE) {
        print_indent(indent + 1);
        printf("NO DEFINITION\n");
    } else {
        print_node_tree(enum_data->entries, indent + 1);
    }
}

void print_struct_stat_tree(Node node, int32_t indent) {
    NodeStructData* struct_stat_data = (NodeStructData*) node.pool_ptr;

    print_indent(indent);
    printf("[STRUCT_STAT]:\n");

    print_node_tree(struct_stat_data->ident, indent + 1);

    if (struct_stat_data->fields.type == NT_NONE) {
        print_indent(indent + 1);
        printf("NOT DEFINED\n");
    } else {
        print_node_tree(struct_stat_data->fields, indent + 1);
    }
}

void print_fn_stat_tree(Node node, int32_t indent) {
    NodeFunctionData* fnstat_data = (NodeFunctionData*) node.pool_ptr;

    print_indent(indent);
    printf("[FN_STAT]:\n");

    print_node_tree(fnstat_data->ident, indent + 1);
    print_node_tree(fnstat_data->params, indent + 1);
    print_node_tree(fnstat_data->type, indent + 1);
    print_node_tree(fnstat_data->block, indent + 1);
}

void print_if_stat_tree(Node node, int32_t indent) {
    NodeIfStatData* ifstat_data = (NodeIfStatData*) node.pool_ptr;

    print_indent(indent);
    printf("[IF_STAT]:\n");

    print_node_tree(ifstat_data->expr, indent + 1);
    print_node_tree(ifstat_data->block, indent + 1);

    if (ifstat_data->ifelse.type != NT_NONE) {
        print_node_tree(ifstat_data->ifelse, indent + 1);
    }
}

void print_while_stat_tree(Node node, int32_t indent) {
    NodeWhileData* whilestat_data = (NodeWhileData*) node.pool_ptr;

    print_indent(indent);
    printf("[WHILE_STAT]:\n");

    print_node_tree(whilestat_data->expr, indent + 1);
    print_node_tree(whilestat_data->block, indent + 1);
}

void print_switch_stat_tree(Node node, int32_t indent) {
    NodeSwitchData* switch_data = (NodeSwitchData*) node.pool_ptr;

    print_indent(indent + 1);
    printf("[SWITCH_STAT]:\n");

    print_node_tree(switch_data->lookup, indent + 1);
    print_node_tree(switch_data->block, indent + 1);
}



void print_array_expr_tree(Node node, int32_t indent) {
    NodeArrayExprData* array_expr_data = (NodeArrayExprData*) node.pool_ptr;

    print_indent(indent);
    printf("[ARRAY_EXPR]:\n");

    for (int32_t i = 0; i < array_expr_data->count; ++i) {
        print_node_tree(array_expr_data->nodes[i], indent + 1);
    }
}

void print_assign_expr_tree(Node node, int32_t indent) {
    NodeAssignExprData* assign_expr_data = (NodeAssignExprData*) node.pool_ptr;
    print_indent(indent);

    printf("[ASSIGN_EXPR]:\n");

    print_indent(indent + 1);
    printf("%s\n", TokenTypeNames[assign_expr_data->op]);

    print_node_tree(assign_expr_data->ident, indent + 1);
    print_node_tree(assign_expr_data->value, indent + 1);
}

void print_bin_expr_tree(Node node, int32_t indent) {
    NodeBinExprData* binexpr_data = (NodeBinExprData*) node.pool_ptr;

    print_indent(indent);
    printf("[BIN_EXPR]:\n");

    print_indent(indent + 1);
    printf("%s\n", TokenTypeNames[binexpr_data->op]);

    print_node_tree(binexpr_data->left, indent + 1);
    print_node_tree(binexpr_data->right, indent + 1);
}

void print_unary_expr_tree(Node node, int32_t indent) {
    NodeUnaryExprData* unaryexpr_data = (NodeUnaryExprData*) node.pool_ptr;

    print_indent(indent);
    printf("[UNARY_EXPR]:\n");

    print_indent(indent + 1);
    printf("%s\n", TokenTypeNames[unaryexpr_data->op]);

    print_node_tree(unaryexpr_data->expr, indent + 1);
}

void print_update_expr_tree(Node node, int32_t indent) {
    NodeUpdateExprData* updateexpr_data = (NodeUpdateExprData*) node.pool_ptr;

    print_indent(indent);
    printf("[UPDATE_EXPR]:\n");

    print_indent(indent + 1);
    printf("%s\n", TokenTypeNames[updateexpr_data->op]);

    print_indent(indent + 1);
    printf("%s\n", updateexpr_data->prefix == true ? "PREFIXED" : "NOT PREFIXED");

    print_node_tree(updateexpr_data->expr, indent + 1);
}

void print_call_expr_tree(Node node, int32_t indent) {
    NodeCallExprData* call_expr_data = (NodeCallExprData*) node.pool_ptr;

    print_indent(indent);
    printf("[CALL_EXPR]:\n");

    print_node_tree(call_expr_data->member, indent + 1);
    print_node_tree(call_expr_data->args, indent + 1);
}

void print_member_expr_tree(Node node, int32_t indent) {
    NodeMemberExprData* member_expr_data = (NodeMemberExprData*) node.pool_ptr;

    print_indent(indent);
    printf("[MEMBER_EXPR]:\n");

    print_node_tree(member_expr_data->object, indent + 1);
    print_node_tree(member_expr_data->property, indent + 1);
}




void print_block_node_tree(Node node, int32_t indent) {
    NodeBlockData* blockstat_data = (NodeBlockData*) node.pool_ptr;

    print_indent(indent);
    printf("[BLOCK_STAT]:\n");

    for (int32_t i = 0; i < blockstat_data->count; ++i) {
        print_node_tree(blockstat_data->nodes[i], indent + 1);
    }
}

void print_param_list_node_tree(Node node, int32_t indent) {
    NodeParamListData* param_list_data = (NodeParamListData*) node.pool_ptr;

    print_indent(indent);
    printf("[PARAMLIST]:\n");

    for (int32_t i = 0; i < param_list_data->count; ++i) {
        print_node_tree(param_list_data->nodes[i], indent + 1);
    }
}

void print_param_node_tree(Node node, int32_t indent) {
    NodeParamData* param_data = (NodeParamData*) node.pool_ptr;

    print_indent(indent);
    printf("[PARAM]:\n");

    print_node_tree(param_data->name, indent + 1);
    print_node_tree(param_data->type, indent + 1);
}

void print_enum_entry_list_node_tree(Node node, int32_t indent) {
    NodeEnumEntryListData* enum_entry_list_data = (NodeEnumEntryListData*) node.pool_ptr;

    print_indent(indent);
    printf("[ENUM_ENTRY_LIST]:\n");

    for (int32_t i = 0; i < enum_entry_list_data->count; ++i) {
        print_node_tree(enum_entry_list_data->nodes[i], indent + 1);
    }
}

void print_enum_entry_node_tree(Node node, int32_t indent) {
    NodeEnumEntryData* enum_entry_data = (NodeEnumEntryData*) node.pool_ptr;

    print_indent(indent);
    printf("[ENUM_ENTRY]:\n");

    print_node_tree(enum_entry_data->name, indent + 1);

    if (enum_entry_data->value.type == NT_NONE) {
        print_indent(indent + 1);
        printf("NO DEFINITION\n");
    } else {
        print_node_tree(enum_entry_data->value, indent + 1);
    }
}

void print_field_list_node_tree(Node node, int32_t indent) {
    NodeFieldListData* field_list_data = (NodeFieldListData*) node.pool_ptr;

    print_indent(indent);
    printf("[FIELDLIST]:\n");

    for (int32_t i = 0; i < field_list_data->count; ++i) {
        print_node_tree(field_list_data->nodes[i], indent + 1);
    }
}

void print_field_node_tree(Node node, int32_t indent) {
    NodeFieldData* field_data = (NodeFieldData*) node.pool_ptr;

    print_indent(indent);
    printf("[FIELD]:\n");

    print_node_tree(field_data->name, indent + 1);
    print_node_tree(field_data->type, indent + 1);
}

void print_arg_list_node_tree(Node node, int32_t indent) {
    NodeArgListData* arg_list_data = (NodeArgListData*) node.pool_ptr;

    print_indent(indent);
    printf("[ARGLIST]:\n");

    for (int32_t i = 0; i < arg_list_data->count; ++i) {
        print_node_tree(arg_list_data->nodes[i], indent + 1);
    }
}

void print_arg_node_tree(Node node, int32_t indent) {
    NodeArgData* arg_data = (NodeArgData*) node.pool_ptr;

    print_indent(indent);
    printf("[ARG]:\n");

    print_node_tree(arg_data->expr, indent + 1);
}

void print_array_type_tree(Node node, int32_t indent) {
    NodeArrayTypeData* array_type_data = (NodeArrayTypeData*) node.pool_ptr;

    print_indent(indent);
    printf("[ARRAY_TYPE]:\n");

    if (array_type_data->size.type == NT_NONE) {
        print_indent(indent + 1);
        printf("NO SIZE\n");
    } else {
        print_node_tree(array_type_data->size, indent + 1);
    }

    print_node_tree(array_type_data->type, indent + 1);
}

void print_switch_entry_list_node_tree(Node node, int32_t indent) {
    NodeSwitchEntryListData* switch_entry_list_data = (NodeSwitchEntryListData*) node.pool_ptr;

    print_indent(indent);
    printf("[SWITCH_ENTRY_LIST]:\n");

    for (int32_t i = 0; i < switch_entry_list_data->count; ++i) {
        print_node_tree(switch_entry_list_data->entries[i], indent + 1);
    }
}

void print_switch_entry_node_tree(Node node, int32_t indent) {
    NodeSwitchEntryData* switch_entry_data = (NodeSwitchEntryData*) node.pool_ptr;

    print_indent(indent);
    printf("[SWITCH_ENTRY]:\n");

    if (switch_entry_data->is_default) {
        print_indent(indent + 1);
        printf("DEFAULT\n");
    } else {
        print_indent(indent + 1);
        printf("CASE\n");
    }

    if (switch_entry_data->expr.type != NT_NONE) {
        print_node_tree(switch_entry_data->expr, indent + 1);
    }

    print_node_tree(switch_entry_data->block, indent + 1);
}




void print_integer_lit_tree(Node node, int32_t indent) {
    NodeLiteralData* lit_data = (NodeLiteralData*) node.pool_ptr;

    print_indent(indent);
    printf("[INTEGER_LIT]: %s\n", lit_data->value);
}

void print_float_lit_tree(Node node, int32_t indent) {
    NodeLiteralData* lit_data = (NodeLiteralData*) node.pool_ptr;

    print_indent(indent);
    printf("[FLOAT_LIT] %s\n", lit_data->value);
}

void print_string_lit_tree(Node node, int32_t indent) {
    NodeLiteralData* lit_data = (NodeLiteralData*) node.pool_ptr;

    print_indent(indent);
    printf("[STRING_LIT]: \"%s\"\n", lit_data->value);
}

void print_ident_lit_tree(Node node, int32_t indent) {
    NodeLiteralData* lit_data = (NodeLiteralData*) node.pool_ptr;

    print_indent(indent);
    printf("[IDENT_LIT]: \"%s\"\n", lit_data->value);
}
