#ifndef TRANSLATOR_TABLE_HPP_INCLUDED
#define TRANSLATOR_TABLE_HPP_INCLUDED

#include <stdio.h>
#include "../lex_tree/tree.hpp"

struct Variable
{
    const char *name;
    size_t number;
};

struct Var_table
{
    size_t size     = 0;
    size_t capacity = 0;
    Variable *table;
};

struct Function
{
    const char *name;
    size_t label;
    Tree_node  *fun_pointer;
    Var_table  *variables;
};

struct Fun_table
{
    size_t size = 0;
    size_t capacity = 0;
    Function *table;
};



struct Translator 
{
    int local_level = 1;
    int mul_assgnmnt = 1;
    Var_table *variables;
    Fun_table *functions;
};


#endif