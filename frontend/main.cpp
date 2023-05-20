#include <malloc.h>
#include "../include/reader.hpp"
#include "../lex_tree/tree.hpp"
#include "../include/read_st.hpp"
#include "../include/ir_creator.hpp"


const char *assemble_file_name = "out/assemble.txt";
const char *output_file_name   = "programs/test_out_tree.std";
const char *ir_file_name       = "out/IR.bc";


int main (int argc, const char *argv[])
{
    Text prog_code = {};
    Root tree_root = {};
    Translator translator = {};

    Translator_ctor (&translator);

    int errors = count_and_read (argc, argv, &prog_code, '\0');
    printf ("%d", errors);
    create_pointers (&prog_code);

    Tree_tokens *tokens = Tokenizer (&prog_code);

    Print_tokens (tokens);

    tree_root.first_node = Get_grammar (tokens, &translator);    
    Graph_print_tree (&tree_root);

    Create_IR (&tree_root, ir_file_name);
    Print_tables (&translator);

}