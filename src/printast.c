#include "parser.h"
#include <ast.h>
#include <stdio.h>
#include <string.h>

const PrintNodeTree_Func PrintNodeTree_Funcs[] = {
    [NT_PROGRAM] = print_program_node_tree,

    [NT_RETURN_STAT] = print_return_stat_tree,
    [NT_BREAK_STAT] = print_break_stat_tree,
    [NT_CONTINUE_STAT] = print_continue_stat_tree,
    [NT_VAR_STAT] = print_var_stat_tree,
    [NT_ENUM_STAT] = print_enum_stat_tree,
    [NT_STRUCT_STAT] = print_struct_stat_tree,
    [NT_FUNC_STAT] = print_fn_stat_tree,
    [NT_IF_STAT] = print_if_stat_tree,
    [NT_WHILE_STAT] = print_while_stat_tree,
    [NT_SWITCH_STAT] = print_switch_stat_tree,

    [NT_BLOCK] = print_block_node_tree,
    [NT_PARAMETER] = print_param_node_tree,
    [NT_ENUM_ENTRY] = print_enum_entry_node_tree,
    [NT_FIELD] = print_field_node_tree,
    [NT_ARGUMENT] = print_arg_node_tree,
    [NT_ARRAY_TYPE] = print_array_type_tree,
    [NT_SWITCH_CASE] = print_switch_case_node_tree,

    [NT_ASSIGN_EXPR] = print_assign_expr_tree,
    [NT_BIN_EXPR] = print_bin_expr_tree,
    [NT_UN_EXPR] = print_unary_expr_tree,
    [NT_UPDATE_EXPR] = print_update_expr_tree,
    [NT_CALL_EXPR] = print_call_expr_tree,
    [NT_MEMBER_EXPR] = print_member_expr_tree,

    [NT_INTEGER_LIT] = print_integer_lit_tree,
    [NT_FLOAT_LIT] = print_float_lit_tree,
    [NT_STRING_LIT] = print_string_lit_tree,
    [NT_IDENT_LIT] = print_ident_lit_tree,
    [NT_ARRAY_LIT] = print_array_lit_tree,
};

void print_indent(int32_t indent) {
    for (int32_t i = 0; i < indent * 2; ++i) {
        printf(" ");
    }
}



void print_node_tree(Node* node, int32_t indent) {
    if (node->type < 0 || node->type >= (sizeof(PrintNodeTree_Funcs) / sizeof(PrintNodeTree_Func[0]))) {
        printf("WARNING: %s not handled in print_node_tree\n", NodeTypeNames[node->type]);
        return;
    } else if (PrintNodeTree_Funcs[node->type] == NULL) {
        printf("WARNING: %s not handled in print_node_tree\n", NodeTypeNames[node->type]);
        return;
    }

    PrintNodeTree_Funcs[node->type](node, indent);
}

void print_node_array(NodeArr* narr, int32_t indent) {
    for (int32_t i = 0; i < narr->count; ++i) {
        print_node_tree(&narr->nodes[i], indent);
    }
}



void print_program_node_tree(Node* node, int32_t indent) {
    NProgram program = node->data.program;

    printf("Program:\n");

    print_indent(indent + 1);
    printf("Body:\n");

    for (int32_t i = 0; i < program.nodes.count; ++i) {
        print_node_tree(&program.nodes.nodes[i], indent + 2);
    }
}




void print_return_stat_tree(Node* node, int32_t indent) {
    NRetStat ret_stat = node->data.ret_stat;

    print_indent(indent);
    printf("RetStat:\n");

    print_node_tree(ret_stat.expr, indent + 1);
}

void print_break_stat_tree(Node* node, int32_t indent) {
    print_indent(indent);
    printf("BreakStat\n");
}

void print_continue_stat_tree(Node* node, int32_t indent) {
    print_indent(indent);
    printf("ContinueStat\n");
}

void print_var_stat_tree(Node* node, int32_t indent) {
    NVarStat var_stat = node->data.var_stat;

    print_indent(indent);
    printf("VarStat:\n");

    print_indent(indent + 1);
    printf("constant: %s\n", var_stat.constant ? "true" : "false");

    print_node_tree(var_stat.ident, indent + 1);
    print_node_tree(var_stat.type, indent + 1);

    if (var_stat.value == NULL) {
        print_indent(indent + 1);
        printf("definition: not defined\n");
    } else {
        print_node_tree(var_stat.value, indent + 1);
    }
}

void print_enum_stat_tree(Node* node, int32_t indent) {
    NEnumStat enum_stat = node->data.enum_stat;

    print_indent(indent);
    printf("EnumStat:\n");

    print_node_tree(enum_stat.ident, indent + 1);

    if (enum_stat.entries.count == 0) {
        print_indent(indent + 1);
        printf("definition: not defined\n");
    } else {
        print_node_array(&enum_stat.entries, indent + 1);
    }
}

void print_struct_stat_tree(Node* node, int32_t indent) {
    NStructStat struct_stat = node->data.struct_stat;

    print_indent(indent);
    printf("StructStat:\n");

    print_node_tree(struct_stat.ident, indent + 1);

    if (struct_stat.fields.count == 0) {
        print_indent(indent + 1);
        printf("definition: not defined\n");
    } else {
        print_node_array(&struct_stat.fields, indent + 1);
    }
}

void print_fn_stat_tree(Node* node, int32_t indent) {
    NFuncStat fn_stat = node->data.func_stat;

    print_indent(indent);
    printf("FnStat:\n");

    print_node_tree(fn_stat.ident, indent + 1);

    print_indent(indent + 1);
    printf("Parameters:\n");
    print_node_array(&fn_stat.params, indent + 2);

    print_node_tree(fn_stat.type, indent + 1);
    print_node_tree(fn_stat.body, indent + 1);
}

void print_if_stat_tree(Node* node, int32_t indent) {
    NIfStat if_stat = node->data.if_stat;

    print_indent(indent);
    printf("IfStat:\n");

    print_node_tree(if_stat.condition, indent + 1);
    print_node_tree(if_stat.body, indent + 1);

    if (if_stat.alternate != NULL) {
        print_node_tree(if_stat.alternate, indent + 1);
    }
}

void print_while_stat_tree(Node* node, int32_t indent) {
    NWhileStat while_stat = node->data.while_stat;

    print_indent(indent);
    printf("WhileStat:\n");

    print_node_tree(while_stat.condition, indent + 1);
    print_node_tree(while_stat.body, indent + 1);
}

void print_switch_stat_tree(Node* node, int32_t indent) {
    NSwitchStat switch_stat = node->data.switch_stat;

    print_indent(indent + 1);
    printf("SwitchStat:\n");

    print_node_tree(switch_stat.lookup, indent + 1);

    print_indent(indent + 1);
    printf("Cases:\n");
    print_node_array(&switch_stat.cases, indent + 2);
}



void print_assign_expr_tree(Node* node, int32_t indent) {
    NAssignExpr assign_expr = node->data.assign_expr;

    print_indent(indent);
    printf("AssignExpr:\n");

    print_indent(indent + 1);
    printf("Operator: %s\n", TokenTypeNames[assign_expr.op]);

    print_node_tree(assign_expr.ident, indent + 1);
    print_node_tree(assign_expr.value, indent + 1);
}

void print_bin_expr_tree(Node* node, int32_t indent) {
    NBinExpr bin_expr = node->data.bin_expr;

    print_indent(indent);
    printf("BinExpr:\n");

    print_indent(indent + 1);
    printf("Operator: %s\n", TokenTypeNames[bin_expr.op]);

    print_node_tree(bin_expr.left, indent + 1);
    print_node_tree(bin_expr.right, indent + 1);
}

void print_unary_expr_tree(Node* node, int32_t indent) {
    NUnExpr un_expr = node->data.un_expr;

    print_indent(indent);
    printf("UnExpr:\n");

    print_indent(indent + 1);
    printf("Operator: %s\n", TokenTypeNames[un_expr.op]);

    print_node_tree(un_expr.expr, indent + 1);
}

void print_update_expr_tree(Node* node, int32_t indent) {
    NUpdateExpr update_expr = node->data.update_expr;

    print_indent(indent);
    printf("UpdateExpr:\n");

    print_indent(indent + 1);
    printf("Operator: %s\n", TokenTypeNames[update_expr.op]);

    print_indent(indent + 1);
    printf("Prefixed: %s\n", update_expr.prefixed == true ? "true" : "false");

    print_node_tree(update_expr.expr, indent + 1);
}

void print_call_expr_tree(Node* node, int32_t indent) {
    NCallExpr call_expr = node->data.call_expr;

    print_indent(indent);
    printf("CallExpr:\n");

    print_node_tree(call_expr.member, indent + 1);

    print_indent(indent + 1);
    printf("Arguments:\n");
    print_node_array(&call_expr.args, indent + 2);
}

void print_member_expr_tree(Node* node, int32_t indent) {
    NMemberExpr member_expr = node->data.member_expr;

    print_indent(indent);
    printf("MemberExpr:\n");

    print_node_tree(member_expr.object, indent + 1);
    print_node_tree(member_expr.property, indent + 1);
}




void print_block_node_tree(Node* node, int32_t indent) {
    NBlock block = node->data.block;

    print_indent(indent);
    printf("Block:\n");

    print_node_array(&block.nodes, indent + 1);
}

void print_param_node_tree(Node* node, int32_t indent) {
    NParameter parameter = node->data.parameter;

    print_indent(indent);
    printf("Parameter:\n");

    print_node_tree(parameter.ident, indent + 1);
    print_node_tree(parameter.type, indent + 1);
}

void print_enum_entry_node_tree(Node* node, int32_t indent) {
    NEnumEntry enum_entry = node->data.enum_entry;

    print_indent(indent);
    printf("EnumEntry:\n");

    print_node_tree(enum_entry.ident, indent + 1);

    if (enum_entry.value == NULL) {
        print_indent(indent + 1);
        printf("Definition: no definition\n");
    } else {
        print_node_tree(enum_entry.value, indent + 1);
    }
}

void print_field_node_tree(Node* node, int32_t indent) {
    NField field = node->data.field;

    print_indent(indent);
    printf("Field:\n");

    print_node_tree(field.ident, indent + 1);
    print_node_tree(field.type, indent + 1);
}

void print_arg_node_tree(Node* node, int32_t indent) {
    NArgument argument = node->data.argument;

    print_indent(indent);
    printf("Argument:\n");

    print_node_tree(argument.expr, indent + 1);
}

void print_array_type_tree(Node* node, int32_t indent) {
    NArrayType array_type = node->data.array_type;

    print_indent(indent);
    printf("ArrayType:\n");

    if (array_type.number == NULL) {
        print_indent(indent + 1);
        printf("Size: not defined\n");
    } else {
        print_node_tree(array_type.number, indent + 1);
    }

    print_node_tree(array_type.type, indent + 1);
}

void print_switch_case_node_tree(Node* node, int32_t indent) {
    NSwitchCase switch_case = node->data.switch_case;

    print_indent(indent);
    printf("SwitchCase:\n");

    if (switch_case.condition == NULL) {
        print_indent(indent + 1);
        printf("DEFAULT\n");
    } else {
        print_indent(indent + 1);
        printf("CASE\n");
    }

    if (switch_case.condition != NULL) {
        print_node_tree(switch_case.condition, indent + 1);
    }

    print_node_tree(switch_case.body, indent + 1);
}




void print_integer_lit_tree(Node* node, int32_t indent) {
    NIntLit int_lit = node->data.int_lit;

    print_indent(indent);
    printf("IntLit: %s\n", int_lit.value);
}

void print_float_lit_tree(Node* node, int32_t indent) {
    NFloatLit float_lit = node->data.float_lit;

    print_indent(indent);
    printf("FloatLit: %s\n", float_lit.value);
}

void print_string_lit_tree(Node* node, int32_t indent) {
    NStringLit string_lit = node->data.string_lit;

    print_indent(indent);
    printf("StringLit: %s\n", string_lit.value);
}

void print_ident_lit_tree(Node* node, int32_t indent) {
    NIdentLit ident_lit = node->data.ident_lit;

    print_indent(indent);
    printf("IdentLit: %s\n", ident_lit.value);
}

void print_array_lit_tree(Node* node, int32_t indent) {
    NArrayLit array_lit = node->data.array_lit;

    print_indent(indent);
    printf("ArrayLit:\n");

    print_node_array(&array_lit.values, indent + 1);
}
