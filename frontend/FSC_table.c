

#include "../include/FSC_table.h"


struct Transition 
{
    enum States state;
    enum OPERATORS value;
    const char *var;
};

struct Transition FSM_table[state_final + 1]['z' + 1] = 
{
    [initial ... state_final] [0 ... 'z'] = {state_final, OP_NON},
    [initial ][' '] = {initial, OP_NON},
    [initial ]['"'] = {state_final, OP_QUOTE},
    [initial ]['*'] = {state_final, OP_MUL},
    [initial ]['/'] = {state_final, OP_DIV},
    [initial ]['('] = {state_final, OP_LBRC},
    [initial ][')'] = {state_final, OP_RBRC},
    [initial ][':'] = {state_final, OP_COLON},
    [initial ][','] = {state_final, OP_COMMA},
    [initial ]['t'] = {state_44, OP_NON},
    [state_44]['a'] = {state_45, OP_NON},
    [state_45]['n'] = {state_final, OP_TAN},
    [initial ]['v'] = {state_1, OP_NON},
    [state_1 ]['a'] = {state_14, OP_NON},
    [state_14]['r'] = {state_final, OP_VAR},
    [initial ]['i'] = {state_2, OP_NON},
    [state_2 ]['f'] = {state_final, OP_IF},
    [state_2 ]['n'] = {state_41, OP_NON},
    [state_41]['p'] = {state_42, OP_NON}, 
    [state_42]['u'] = {state_43, OP_NON},
    [state_43]['t'] = {state_final, OP_INPUT},
    [initial ]['!'] = {state_46,    OP_NON},

    [initial ]['w'] = {state_16, OP_NON},
    [state_16]['h'] = {state_17, OP_NON},
    [state_17]['i'] = {state_18, OP_NON},
    [state_18]['l'] = {state_19, OP_NON},
    [state_19]['e'] = {state_final, OP_WHILE},

    [initial ]['p'] = {state_20, OP_NON},
    [state_20]['r'] = {state_21, OP_NON},
    [state_21]['i'] = {state_22, OP_NON},
    [state_22]['n'] = {state_23, OP_NON},
    [state_23]['t'] = {state_final, OP_PRINT},

    [initial ]['e'] = {state_9, OP_NON},
    [state_9 ]['l'] = {state_25, OP_NON},
    [state_25]['s'] = {state_26, OP_NON},
    [state_26]['e'] = {state_final, OP_ELSE},
    [state_25]['i'] = {state_27, OP_NON},
    [state_27]['f'] = {state_final, OP_ELIF},

    [initial ]['s'] = {state_28, OP_NON},
    [state_28]['q'] = {state_29, OP_NON},
    [state_29]['r'] = {state_30, OP_NON},
    [state_30]['t'] = {state_final, OP_SQRT},

    [initial ]['f'] = {state_31, OP_NON},
    [state_31]['u'] = {state_32, OP_NON},
    [state_32]['n'] = {state_33, OP_NON},
    [state_33]['c'] = {state_final, OP_FUNC},

    [initial ]['r'] = {state_34, OP_NON},
    [state_34]['e'] = {state_35, OP_NON},
    [state_35]['t'] = {state_36, OP_NON},
    [state_36]['u'] = {state_37, OP_NON},
    [state_37]['r'] = {state_38, OP_NON},
    [state_38]['n'] = {state_final, OP_RETURN},

    [initial ]['>'] = {state_final, OP_IS_GT},

    [initial ]['<'] = {state_final, OP_IS_BT},
    [initial ]['='] = {state_13, OP_ASSGN},
    [state_13][0 ... 'z'] = {state_final, OP_ASSGN},
    [state_13]['='] = {state_final, OP_IS_EE},
    [initial ]['+'] = {state_39, OP_ADD},
    [state_39][0 ... 'z'] = {state_final, OP_ADD},
    [state_39]['='] = {state_final, OP_ADD_ASSIGN},
    [initial ]['-'] = {state_40, OP_NON},
    [state_40][0 ... 'z'] = {state_final, OP_SUB},
    [state_40]['='] = {state_final, OP_SUB_ASSIGN},
    [state_46]['='] = {state_final, OP_IS_NE},
};


enum OPERATORS In_table (const char **str)
{
    enum States old_state = initial;
    enum States cur_state = initial;
    char current_signal = ' ';
    const char *temp = *str;

    while (cur_state != state_final && *temp != 0)
    {
        current_signal = *(temp++);
        old_state = cur_state;
        cur_state = FSM_table[cur_state][current_signal].state;
        // prints ("%c: %s: %d: %d\n", current_signal, temp + ind, temp[ind - 1], ind);
        // getc (stdin);
    }
    enum OPERATORS value = FSM_table[old_state][current_signal].value;
    if (value != OP_NON)
        *str = temp;
    if (value == OP_SUB || value == OP_ADD)
        *str = temp - 1;

    return value;
}


