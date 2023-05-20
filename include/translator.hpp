#pragma once

#include <stdio.h>

#include "translator_table.hpp"
#include "emit.hpp"

#define IS_OP(op_type) (node->type == OPERATOR and node->value.op == op_type)
#define IS_OPERATORS(node) node->type == OPERATOR and (node->value.op == OP_ADD or node->value.op == OP_SUB or\
                                                       node->value.op == OP_MUL or node->value.op == OP_DIV or\
                                                       node->value.op == OP_SQRT or node->value.op == OP_SIN)
    
#define IS_COMPARE_OPERATORS(node) node->type == OPERATOR and (node->value.op == OP_IS_EE or node->value.op == OP_IS_NE or\
                                                               node->value.op == OP_IS_BT or node->value.op == OP_IS_GT)

bool Is_in_var_table  (Var_table *vars, const char* var_name);
bool Is_in_fun_table (Fun_table *functions, const char *func_name);
Var_table *Var_table_ctor (void);
void Var_table_push (Var_table *vars, const char *name);
void Var_table_resize (Var_table *variables);
void Fun_table_resize (Fun_table *functions);
void Translator_ctor  (Translator *translator);
const char* Find_function (Fun_table *functions, const char *func_name);
size_t Find_function_label (Fun_table *functions, const char *func_name);

void Translate_to_assembler (const char *assemble_file_name, Root *tree_root, Translator *translator);
void Translate_statement (Execute_module *self, Tree_node *node, Var_table *variables, Fun_table *functions);
void Translate_if (Execute_module *self, Tree_node *node, Var_table *variables, Fun_table *functions);
void Translate_expression (Execute_module *self, Tree_node *node, Var_table *variables, Fun_table *functions);
void Translate_loop (Execute_module *self, Tree_node *node, Var_table *variables, Fun_table *functions);
void Translate_assigment (Execute_module *self, Tree_node *node, Var_table *variables, Fun_table *functions);
void Translate_function (Execute_module *self, Tree_node *node, Var_table *variables, Fun_table *functions);
void Translate_call (Execute_module *self, Tree_node *node, Var_table *variables, Fun_table *functions);
void Translate_functions (Execute_module *self, Translator *trans_table);
void Translate_print (Execute_module *self, Tree_node *node, Var_table *variables, Fun_table *functions);
void Translate_var_declaration (Execute_module *self, Tree_node *node, Var_table *variables, Fun_table *functions);

void Make_translator_table (Tree_node *node, Translator *translator);