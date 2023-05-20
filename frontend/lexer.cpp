#include <assert.h>
#include <string.h>
#include <math.h>
#include "../include/utilities.hpp"
#include "../include/file_analize.hpp"
#include "../include/lexer.hpp"
#include "../lex_tree/tree.hpp"


static Tree_tokens *Tokens_ctor (size_t size)
{
    Tree_tokens *tokens = (Tree_tokens *) Safety_calloc (sizeof (Tree_tokens));
    tokens->capacity = 0;
    tokens->size = 0;
    tokens->array = (Tree_node **) Safety_calloc (sizeof (Tree_node) * size); //fix it, invalid size

    return tokens;
}


static void Skip_spaces (const char **str)
{
    while (**str == ' ')
        *str += 1;
}

static int Add_delim (const char **code_string, long long int *str_len)
{
    int space_number = 4;
    int deep = 0;
    while (strncmp (*code_string, "    ", space_number) == 0)
    {
        *code_string += space_number;
        *str_len     -= space_number;
        deep++;
    }

    return deep;
}


static int Check_deep (Tree_tokens *tokens, int old_deep, int deep)
{
    if (old_deep > deep)
        tokens->array[tokens->capacity++] = New_operator (OP_ENDB, NULL, NULL, 0, 0);
    else if (old_deep < deep)
        tokens->array[tokens->capacity++] = New_operator (OP_BEGINB, NULL, NULL, 0, 0);
    return deep;
}


Tree_tokens *Tokenizer (Text *code)
{

    int old_deep = 0;
    Tree_tokens *tokens = Tokens_ctor (code->number_of_symbols);
    Tree_node   *node   = NULL;

    for (size_t index = 0; index < code->number_of_strings; index++)
    {
        const char *str_ptr = code->string[index].point;
        const char *str_tmp = str_ptr;
        long long int str_len = code->string[index].len;

        int deep = Add_delim (&str_ptr, &str_len);
        old_deep = Check_deep (tokens, old_deep, deep);

        while (*str_ptr != '\0' && *str_ptr != '#')
        {
            Skip_spaces (&str_ptr);
 
            node = Is_num (&str_ptr, index + 1, str_ptr - str_tmp);

            if (node)
                tokens->array[tokens->capacity++] = node;
            else if (*str_ptr != '\0' && *str_ptr != ' ' && *str_ptr != '#')
            {
                printf ("lex error in string %ld\n", tokens->array[tokens->capacity - 1]->str_number);
                break;
            }

        }
        if (str_len > 0 && *str_ptr != '#')
            tokens->array[tokens->capacity++] = New_operator (OP_EOS, NULL, NULL, 0, 0);
    }
    tokens->array[tokens->capacity++] = New_operator (OP_EOF, NULL, NULL, 0, 0);
    
    return tokens;
}





#pragma GCC diagnostic ignored "-Wcast-qual"

Tree_node *Is_num (const char **str, size_t str_num, size_t str_pos)
{
    double number = 0;
    const char *s_old = *str;

    number = strtod (*str, (char **) str);

    if (s_old != *str)
        return New_num (number, str_pos, str_num);
    
    return Is_operator (str, str_num, str_pos);
}

#pragma GCC diagnostic warning "-Wcast-qual"


Tree_node *Is_variable (const char **str, size_t str_num, size_t str_pos)
{
    int size = 0;
    char *var_value = (char *) Safety_calloc (strlen (*str));

    if (*(*str - 1) == '"' )
        sscanf (*str, " %[^\n\"]%n", var_value, &size);
    else    
        sscanf (*str, " %[_A-Za-z0-9]%n", var_value, &size);

    if (size)
    {
        *str += size;
        return New_var (var_value, str_pos, str_num);
    }

    free (var_value);
    return NULL;
}


extern "C" enum OPERATORS In_table (const char **str);

/*
#define DEF_OP(op_name, op_number, op_str, ...)         \
    size = strlen (op_str);                        \
    if (strncmp (*str, op_str, size) == 0)         \
    {                                              \
        *str += size;                              \
        return New_operator (op_name, NULL, NULL, str_pos, str_num); \
    }
*/

Tree_node *Is_operator (const char **str, size_t str_num, size_t str_pos)
{
    assert (str);

    // size_t size = 0;
    OPERATORS value = In_table (str);
    if (value != OP_NON)
        return New_operator (value, NULL, NULL, str_pos, str_num);
    // #include "..\include\operators_dsl.hpp"

    return Is_variable (str, str_num, str_pos);
}
