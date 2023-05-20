#ifndef READ_ST_HPP_INCLUDED
#define READ_ST_HPP_INCLUDED

#include "../lex_tree/tree.hpp"
#include "../include/translator_table.hpp"

#define IS_OP(op_type) (node->type == OPERATOR and node->value.op == op_type)
#define IS_OPERATORS(node) node->type == OPERATOR and (node->value.op == OP_ADD or node->value.op == OP_SUB or\
                                                       node->value.op == OP_MUL or node->value.op == OP_DIV or\
                                                       node->value.op == OP_SQRT or node->value.op == OP_SIN)
    
#define IS_COMPARE_OPERATORS(node) node->type == OPERATOR and (node->value.op == OP_IS_EE or node->value.op == OP_IS_NE or\
                                                               node->value.op == OP_IS_BT or node->value.op == OP_IS_GT)


void Read_tree_from_file (const char *t_file_name, Root *tree_root);
const char *Read_node (const char *source, Tree_node* parent);

void Print_tree_to_standart_file (const char* file_name, Root* tree_root);
void Save_node (FILE* output_file, Tree_node* node, int height);

Tree_node *Is_num (const char *value);
Tree_node *Is_variable (const char *value);
Tree_node *Is_operator (const char *value);
void Remove_trash (Tree_node * node);
void Print_value (FILE* out_file, Tree_node *node, int height);
#endif