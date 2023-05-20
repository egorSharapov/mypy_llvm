#ifndef READER_HPP_INCLUDED
#define READER_HPP_INCLUDED

#include "../lex_tree/tree.hpp"
#include "lexer.hpp"
#include "translator.hpp"

#define TOKEN tokens->array[tokens->size]
#define funct translator->functions->table[translator->functions->size]
#define IS_TYPE(type_val)   tokens->array[tokens->size]->type == type_val
#define IS_TOKEN_OP(op_val) tokens->array[tokens->size]->value.op == op_val
#define IS_NEXT_OP(op_val)  tokens->array[tokens->size + 1]->value.op == op_val

#define CHECK_OP(op_val)    if (IS_TOKEN_OP (op_val)) tokens->size += 1;                                         \
                            else {printf ("error, expected operator %d, but recieved %s: in %s:%ld:%ld in function: reader.cpp:%d\n|"                                 \
                                           "", op_val, tokens->array[tokens->size]->value.id,  "factorial.mypy", TOKEN->str_number, TOKEN->str_position, __LINE__); exit (0);}

#define IS_UNARY()          IS_TOKEN_OP (OP_SIN) or IS_TOKEN_OP (OP_COS) or IS_TOKEN_OP (OP_SQRT) or IS_TOKEN_OP (OP_TAN)
#define IS_ASSIGN_OP()      (IS_TOKEN_OP (OP_SUB_ASSIGN) or IS_TOKEN_OP (OP_ADD_ASSIGN) or IS_TOKEN_OP (OP_ASSGN))


static const int MAX_OP_SIZE = 10;



enum LEVELS
{
    GLOBAL_LEVEL = 1,
    FUNC_LEVEL   = 2,
    LOCAL_LEVEL  = 3,
};



Tree_node* Get_grammar             (Tree_tokens *tokens, Translator *translator);
Tree_node* Get_expression          (Tree_tokens *tokens, Translator *translator);
Tree_node* Get_muldiv_expression   (Tree_tokens *tokens, Translator *translator);
Tree_node* Get_major_expression    (Tree_tokens *tokens, Translator *translator);
Tree_node* Get_number              (Tree_tokens *tokens);
Tree_node* Get_power               (Tree_tokens *tokens, Translator *translator);
Tree_node* Get_variable            (Tree_tokens *tokens, Translator *translator);
Tree_node* Get_function_expression (Tree_tokens *tokens, Translator *translator);
Tree_node *Get_choice_instruction  (Tree_tokens *tokens, Translator *translator); // get insdtrctuion choice
Tree_node *Get_assigment           (Tree_tokens *tokens, Translator *translator);
Tree_node *Get_instruction         (Tree_tokens *tokens, Translator *translator);
Tree_node *Get_var_declaration     (Tree_tokens *tokens, Translator *translator);
Tree_node *Get_loop_instruction    (Tree_tokens *tokens, Translator *translator);
Tree_node *Get_fun_declaration     (Tree_tokens *tokens, Translator *translator);
Tree_node *Get_fun_call            (Tree_tokens *tokens, Translator *translator);
Tree_node *Get_jump_instruction    (Tree_tokens *tokens, Translator *translator);

void Print_tokens (Tree_tokens *tokens);
void Print_tables (Translator *translator);

#endif