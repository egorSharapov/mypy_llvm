#include <malloc.h>
#include <assert.h>
#include <math.h>
#include "../include/reader.hpp"
#include "../include/utilities.hpp"
#include "../include/file_analize.hpp"
#include "../lex_tree/tree.hpp"


/*
grammar -> Instruction EOF
Instruction -> fundeclaration | vardeclaration | assigment | choice_instruction | loop_instruction ?Instruction
assigment -> variable '=' expression | funcall
choice_instruction -> if expression ':' body_instruction
vardeclaration -> 'var' variable = expression
fundeclaration -> 'fun' variable '(' ')' ':' body_instruction 
funcall        -> variable '(' ')' 
jumpinstruction -> 'break' 'continue' 'return'
*/
//таблица имен таблица джампов

Tree_node* Get_grammar (Tree_tokens *tokens, Translator *translator)
{
    assert (tokens);

    Tree_node *left_node = Get_instruction (tokens, translator);

    if (IS_TOKEN_OP (OP_EOF))
        printf ("succes syntax analize\n");

    return left_node;
}

Tree_node *Get_instruction (Tree_tokens *tokens, Translator *translator)
{
    Tree_node* right_node = NULL;
    Tree_node* left_node = NULL;

    if ((IS_TYPE (IDENTIFIER) or IS_TYPE (OPERATOR)) and !(IS_TOKEN_OP (OP_EOF) or IS_TOKEN_OP (OP_ENDB)))
    {
        if (IS_TYPE (IDENTIFIER))
            left_node = Get_assigment (tokens, translator);
        else if (IS_TOKEN_OP (OP_VAR))
            left_node = Get_var_declaration (tokens, translator);
        else if (IS_TOKEN_OP (OP_IF))
            left_node = Get_choice_instruction (tokens, translator);
        else if (IS_TOKEN_OP (OP_WHILE))
            left_node = Get_loop_instruction (tokens, translator);
        else if (IS_TOKEN_OP (OP_FUNC))
            left_node = Get_fun_declaration (tokens, translator);
        else if (IS_TOKEN_OP (OP_PRINT))
            left_node = Get_fun_call (tokens, translator);
        else if (IS_TOKEN_OP (OP_RETURN))
            left_node = Get_jump_instruction (tokens, translator);
        else
            return left_node;

        right_node = Get_instruction (tokens, translator);
        left_node = New_operator (OP_ST, left_node, right_node, 0, 0);
 
    }

    return left_node;
}

Tree_node *Get_jump_instruction (Tree_tokens *tokens, Translator *translator)
{
    if (IS_TOKEN_OP (OP_RETURN))
    {
        Tree_node *node_left = TOKEN;
        tokens->size += 1;
        Tree_node *params = Get_expression (tokens, translator);
        while (IS_TOKEN_OP (OP_COMMA))
        {
            tokens->size += 1;
            params = New_operator (OP_ST, params, NULL, 0, 0);
            params->right = Get_expression (tokens, translator);
        }
        node_left->left = params;
        CHECK_OP (OP_EOS)
        return node_left;
    }
    return NULL;
}

// static Tree_node *Get_params (Tree_tokens *tokens, Translator *translator)
// {
//     Tree_node *params = NULL;
//     if (IS_TOKEN_OP (OP_QUOTE))
//     {
//         tokens->size += 1;
//         params = New_operator (OP_STR, TOKEN, NULL, 0, 0);
//         tokens->size += 1;
//         CHECK_OP (OP_QUOTE)
//     }
//     else
//         params = Get_expression (tokens, translator);

//     if (!(IS_TOKEN_OP (OP_RBRC)))
//     {
//         CHECK_OP (OP_COMMA)
//     }

//     return params;
// }


Tree_node *Get_fun_call (Tree_tokens *tokens, Translator *translator)
{
    if (IS_TOKEN_OP (OP_INPUT))
    {
        Tree_node *fun_call = tokens->array[tokens->size++];
        CHECK_OP (OP_LBRC)
        if (IS_TOKEN_OP (OP_QUOTE))
        {
            tokens->size += 1;
            fun_call->right = New_operator (OP_STR, TOKEN, NULL, 0, 0);
            tokens->size += 1;
            CHECK_OP (OP_QUOTE)
        }
        CHECK_OP (OP_RBRC)

        return fun_call;
    }
    if (IS_TOKEN_OP (OP_PRINT))
    {
        Tree_node *fun_call = tokens->array[tokens->size++];
        CHECK_OP (OP_LBRC)

        // Tree_node *params = Get_params (tokens, translator);
        // if (!(IS_TOKEN_OP (OP_RBRC)))
        // {
        //     params = New_operator (OP_ST, params, NULL, 0, 0);
        //     params->right = Get_params (tokens, translator);
        // }
        // fun_call->left = params;
        // params = params->left;
        // while (!(IS_TOKEN_OP (OP_RBRC)))
        // {
        //     params = New_operator (OP_ST, params, NULL, 0, 0);
        //     params->right = Get_params (tokens, translator);
        //     params = params->right;
        // }


        if (IS_TOKEN_OP (OP_QUOTE))
        {
            tokens->size += 1;
            fun_call->right = New_operator (OP_STR, TOKEN, NULL, 0, 0);
            tokens->size += 1;
            CHECK_OP (OP_QUOTE)
            if (IS_TOKEN_OP (OP_COMMA))
                tokens->size += 1;
        }
        if (!(IS_TOKEN_OP (OP_RBRC)))
            fun_call->left = Get_expression (tokens, translator);

        fun_call->left = New_operator (OP_PARAM, fun_call->left, NULL, 0, 0);
        CHECK_OP (OP_RBRC)
        CHECK_OP (OP_EOS)
        return fun_call;
    }
    if (IS_TYPE (IDENTIFIER))
    {
        Tree_node *fun_call = New_operator (OP_CALL, NULL, NULL, 0, 0);

        fun_call->left = tokens->array[tokens->size++];
        if (!Is_in_fun_table (translator->functions, fun_call->left->value.id))
            printf ("no declaration function: %s\n", fun_call->left->value.id);

        fun_call->left->right = New_operator (OP_TYPE, NULL, NULL, 0, 0);
        CHECK_OP (OP_LBRC)

        Tree_node *params = Get_expression (tokens, translator);
        params = New_operator (OP_PARAM, params, NULL, 0, 0);

        while (IS_TOKEN_OP (OP_COMMA))
        {
            tokens->size += 1;
            params = New_operator (OP_PARAM, NULL, params, 0, 0);
            params->left = Get_expression (tokens, translator);
        }
        if (params)
            fun_call->left->left = params;


        CHECK_OP (OP_RBRC)

        return fun_call;
    }
    return NULL;
}

static Tree_node *Get_instruction_body(Tree_tokens *tokens, Translator *translator)
{
        CHECK_OP (OP_COLON)
        CHECK_OP (OP_EOS)
        CHECK_OP (OP_BEGINB)

        Tree_node *body = Get_instruction (tokens, translator);

        CHECK_OP (OP_ENDB) 

        return body;
}


Tree_node *Get_fun_declaration (Tree_tokens *tokens, Translator *translator)
{
    if (translator->local_level != GLOBAL_LEVEL)
    {
        printf ("declaration function in function\n");
        abort ();
    }

    if (IS_TOKEN_OP (OP_FUNC))
    {
        //создание таблички перееменных функции
        Var_table *variables = Var_table_ctor ();

        Tree_node *func = tokens->array[tokens->size];

        tokens->size += 1;
        Fun_table_resize (translator->functions);

        func->left = tokens->array[tokens->size++];

        func->left->right = New_operator (OP_TYPE, NULL, NULL, 0, 0);

        funct.fun_pointer = func;
        funct.name = func->left->value.id;
        funct.variables = variables;

        CHECK_OP(OP_LBRC);

        Tree_node *params = func->left->left;

        if (IS_TOKEN_OP (OP_VAR))
        {
            params = New_operator (OP_PARAM, TOKEN, NULL, 0, 0);
            func->left->left = params;
            tokens->size += 1;
            params->left->left = tokens->array[tokens->size++];
            Var_table_push (variables, params->left->left->value.id);
            while (IS_TOKEN_OP (OP_COMMA))
            {
                tokens->size += 1;
                params->right = New_operator (OP_PARAM, NULL, NULL, 0, 0);
                params = params->right;
                params->left = tokens->array[tokens->size++];
                params->left->left = tokens->array[tokens->size++];
                Var_table_push (variables, params->left->left->value.id);
            }
        }
        CHECK_OP (OP_RBRC);
    
        translator->local_level = FUNC_LEVEL;
        func->right = Get_instruction_body (tokens, translator);
        translator->local_level = GLOBAL_LEVEL;
        translator->functions->size += 1;

        return func;
    }
    return NULL;
}


Tree_node *Get_var_declaration (Tree_tokens *tokens, Translator *translator)
{

    if (IS_TYPE (OPERATOR) and IS_TOKEN_OP (OP_VAR))
    {
        Var_table *vars = NULL;

        Tree_node *var_node = TOKEN;
        tokens->size += 1;
        const char *temp_name = tokens->array[tokens->size]->value.id;
        if (translator->local_level == GLOBAL_LEVEL)
            vars = translator->variables;
        if (translator->local_level == FUNC_LEVEL)
            vars = translator->functions->table[translator->functions->size].variables;

        if (Is_in_var_table (vars, temp_name))
            printf ("warning: redeclaration var %s\n", temp_name);
    
        Var_table_push (vars, temp_name);

        var_node->left = Get_variable (tokens, translator);
        CHECK_OP (OP_ASSGN)

        var_node->right = Get_expression (tokens, translator);

        CHECK_OP (OP_EOS);
        return var_node;

    }
    return NULL;
}


static Tree_node *Get_choice_comparsion (Tree_tokens *tokens, Translator *translator)
{
    Tree_node *compound_op = Get_expression (tokens, translator);
    
    if (IS_TOKEN_OP (OP_IS_BT)   or IS_TOKEN_OP (OP_IS_GT) or
        IS_TOKEN_OP (OP_IS_NE) or IS_TOKEN_OP (OP_IS_EE))
    {
        Tree_node *compare_op = TOKEN;
        tokens->size += 1; 
        compare_op->left = compound_op;
        compare_op->right = Get_expression (tokens, translator);
        compound_op = compare_op;
    }
    return compound_op;
}


Tree_node *Get_loop_instruction (Tree_tokens *tokens, Translator *translator)
{
    if (IS_TYPE (OPERATOR) and IS_TOKEN_OP (OP_WHILE))
    {
        Tree_node *parent = tokens->array[tokens->size++];

        parent->left = Get_choice_comparsion (tokens, translator);

        // printf ("operator: %d\n", tokens->array[tokens->size]->op_value);

        CHECK_OP (OP_COLON)
        CHECK_OP (OP_EOS)
        CHECK_OP (OP_BEGINB)

        parent->right = Get_instruction (tokens, translator);

        CHECK_OP (OP_ENDB)

        return parent;

    }
    return NULL;

}


Tree_node *Get_choice_instruction (Tree_tokens *tokens, Translator *translator)
{
    if (IS_TYPE (OPERATOR) and IS_TOKEN_OP (OP_IF))
    {
        Tree_node *choice_op = TOKEN;
        Tree_node *tmp_op = choice_op;
        tokens->size += 1;

        choice_op->left  = Get_choice_comparsion (tokens, translator);
        choice_op->right = Get_instruction_body (tokens, translator);

        while (IS_TOKEN_OP (OP_ELIF))
        {
            free(TOKEN);
            tokens->size += 1;
            Tree_node * elif_op = New_operator (OP_ELSE, NULL, NULL, 0, 0);
            elif_op->left  = choice_op->right;
            elif_op->right = New_operator      (OP_IF, NULL, NULL, 0, 0);

            elif_op->right->left  = Get_choice_comparsion (tokens, translator);
            elif_op->right->right = Get_instruction_body  (tokens, translator);

            choice_op->right = elif_op;
            choice_op = elif_op->right;
        }
        if (IS_TOKEN_OP (OP_ELSE))
        {
            Tree_node *else_op = TOKEN;
            tokens->size += 1;
            else_op->right = Get_instruction_body (tokens, translator);
            
            else_op->left    = choice_op->right;
            choice_op->right = else_op;
        }
        return tmp_op;
    }
    return NULL;
}

Tree_node *Get_assigment (Tree_tokens *tokens, Translator *translator)
{
    Tree_node* node_left = Get_variable (tokens, translator);
    Tree_node *params = node_left;
    if (IS_TOKEN_OP (OP_COMMA))
    {
        tokens->size += 1;
        params = New_operator (OP_ST, params, NULL, 0, 0);
        node_left = params;
        params->right = Get_variable (tokens, translator);
    }
    while (IS_TOKEN_OP (OP_COMMA))
    {
        tokens->size += 1;
        params->right = New_operator (OP_ST, params->right, NULL, 0, 0);
        params = params->right;
        params->right = Get_variable (tokens, translator);
    }

    if (IS_TYPE (OPERATOR) and IS_ASSIGN_OP())
    {
        Tree_node *assign = tokens->array[tokens->size++];

        assign->right = Get_expression (tokens, translator);
        assign->left = node_left;

        node_left = assign;
    }
    CHECK_OP (OP_EOS)

    return node_left;
}

Tree_node* Get_expression (Tree_tokens *tokens, Translator *translator)
{
    Tree_node* node_left = Get_muldiv_expression (tokens, translator);

    while (IS_TYPE (OPERATOR) and (IS_TOKEN_OP (OP_ADD) or IS_TOKEN_OP (OP_SUB)))
    {
        Tree_node *addsub_node = tokens->array[tokens->size++];

        addsub_node->right = Get_muldiv_expression (tokens, translator);
        addsub_node->left = node_left;

        node_left = addsub_node;
    }
    return node_left;
}


Tree_node* Get_muldiv_expression (Tree_tokens *tokens, Translator *translator)
{
    Tree_node* node_left = Get_power (tokens, translator);

    while (IS_TYPE (OPERATOR) and (IS_TOKEN_OP (OP_MUL) or IS_TOKEN_OP (OP_DIV)))
    {
        Tree_node *muldiv_node  = tokens->array[tokens->size++];
        muldiv_node->right = Get_power (tokens, translator);

        muldiv_node->left = node_left;
        node_left = muldiv_node;
    }
    return node_left;
}


Tree_node* Get_major_expression (Tree_tokens *tokens, Translator *translator)
{
    Tree_node* val = NULL;

    if (IS_TYPE (OPERATOR) and IS_TOKEN_OP (OP_LBRC))
    {
        free (TOKEN);
        tokens->size += 1;
        val = Get_expression (tokens, translator);
        CHECK_OP (OP_RBRC)
    }
    else if (IS_TOKEN_OP (OP_SUB))
    {
        val = TOKEN;
        tokens->size += 1;
        val->left = New_num (0, 0, 0);
        val->right = Get_expression (tokens, translator);
    }
    else if (IS_TYPE (NUMBER))
        val = Get_number (tokens);
    else if (IS_TYPE (IDENTIFIER))
    {
        if (IS_NEXT_OP (OP_LBRC))
            val = Get_fun_call (tokens, translator);
        else
            val = Get_variable (tokens, translator);
    }
    else if (IS_TOKEN_OP (OP_INPUT))
        val = Get_fun_call (tokens, translator);
    else
    {
        CHECK_OP (OP_NON);
    }

    return val;
}


Tree_node* Get_number (Tree_tokens *tokens)
{
    if (IS_TYPE (NUMBER))
        return tokens->array[tokens->size++];
    else
        printf ("get number error");

    return NULL;
}

Tree_node* Get_variable (Tree_tokens *tokens, Translator *translator)
{
    if (IS_TYPE (IDENTIFIER))
    {
        const char *var_name = tokens->array[tokens->size]->value.id;
        Var_table * vars = NULL;
        if (translator->local_level == GLOBAL_LEVEL)
            vars = translator->variables;

        if (translator->local_level == FUNC_LEVEL)
            vars = translator->functions->table[translator->functions->size].variables;
        
        if (Is_in_var_table (vars, var_name))
            return tokens->array[tokens->size++];
        else
        {
            printf ("no declaration var: %s in file factorial.mypy:%ld:%ld\n", var_name, 
                                                                   tokens->array[tokens->size]->str_number,
                                                                       tokens->array[tokens->size]->str_position);
            exit (0);
        }
    }
    else
        printf ("get variable error");
    
    return NULL;
}


Tree_node* Get_power (Tree_tokens *tokens, Translator *translator)
{
    Tree_node* node_left = Get_function_expression (tokens, translator);
    
    if (IS_TYPE (OPERATOR) and IS_TOKEN_OP (OP_PWR))
    {
        Tree_node *pwr = tokens->array[tokens->size++];

        pwr->right = Get_function_expression (tokens, translator);
        pwr->left = node_left;

        return pwr;
    }
    return node_left;
}


Tree_node *Get_function_expression (Tree_tokens *tokens, Translator *translator)
{
    if (IS_TYPE (OPERATOR) and (IS_UNARY()))
    {
        Tree_node  *func_operator = tokens->array[tokens->size++];
        CHECK_OP (OP_LBRC);

        func_operator->left = Get_expression (tokens, translator);
        CHECK_OP (OP_RBRC);
        return func_operator;
    }

    return Get_major_expression (tokens, translator);
}






void Print_tokens (Tree_tokens *tokens)
{
    printf ("%ld\n", tokens->capacity);
    for (size_t index = 0; index < tokens->capacity; index++)
    {
        printf ("tp: %2d ", tokens->array[index]->type);
        Tree_node *elem = tokens->array[index];
        switch (elem->type)
        {
        case OPERATOR:
            printf ("| op: %s", convert_graph_op(elem->value.op));
            break;

        case NUMBER:
            printf ("| num:%02lg", elem->value.num);
            break;

        case IDENTIFIER:
            printf ("| var: %s", elem->value.id);
            break;

        default:
            printf ("|non   ");
            break;
        }
        printf ("\n");
    }

}

static void Print_var_table (Var_table *variables)
{
    for (size_t index = 0; index < variables->size; index++)
    {
        printf ("   variable: %s:%ld\n", variables->table[index].name, index);
    }
}

void Print_tables (Translator *translator)
{
    printf ("\nglobal:\n");
    Print_var_table (translator->variables);

    for (size_t index = 0; index < translator->functions->size; index++)
    {
        printf ("function %s:%ld\n", translator->functions->table[index].name, index);
        Print_var_table (translator->functions->table[index].variables);
    }
}