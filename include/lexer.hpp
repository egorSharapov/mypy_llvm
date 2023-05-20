#pragma once

#include "../lex_tree/tree.hpp"
#include "../include/file_analize.hpp"


typedef struct Tree_tokens
{
    size_t size;
    size_t capacity;
    Tree_node **array;
    Tree_node **current;
} Tree_tokens;


Tree_tokens *Tokenizer (Text *code);
Tree_node *Is_num      (const char **str, size_t str_num, size_t str_pos);
Tree_node *Is_variable (const char **str, size_t str_num, size_t str_pos);
Tree_node *Is_operator (const char **str, size_t str_num, size_t str_pos);
Tree_node *Test_is_operator  (const char **str, size_t str_num, size_t str_pos);