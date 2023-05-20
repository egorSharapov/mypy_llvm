#ifndef SIMPLIFICATION_HPP_INCLUDED
#define SIMPLIFICATION_HPP_INCLUDED

#include "../include/utilities.hpp"

#define TYPE_IS(son, node_type) node->son->type == node_type

#define FREE_AND_MODIFICATE(free_son, swap_son) \
{                                     \
    Tree_node *temp = node->swap_son; \
    free (node->free_son);            \
    free (node);                      \
    node = temp;                      \
    *is_modifed = true;               \
}

#define OP_IS(type) node->value.op == type
#define IS_VAL(son, val) is_equal (node->son->value.num, val)


bool is_unary (Tree_node *node);
Tree_node *Constant_simplification(Tree_node *node, bool *is_modifed);
Tree_node *Simplification(Tree_node *node);

#endif