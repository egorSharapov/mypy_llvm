#include "../include/reader.hpp"
#include "../include/read_st.hpp"
#include "../include/simplification.hpp"
#include "../include/ir_creator.hpp"


static int local_vars_number = 0;

static llvm::Value *Find_var (Names_t *names, const char *value)
{
    for (size_t i = 0; i < names->number; i++)
    {
        puts (names->variables[i].name);
        if (strcmp (names->variables[i].name, value) == 0)
            return names->variables[i].node;
    }
    puts ("unfind");
    return nullptr;
}


static void Create_print (Creator_t *self, Tree_node *node)
{
    llvm::Function *call_f = self->TheModule->getFunction ("print");

    llvm::Value *arg = Create_expression (self, node->left->left);
    self->builder->CreateCall (call_f, arg);
}


static llvm::Value *Create_sqrt (Creator_t *self, Tree_node *node)
{
    llvm::Function *call_f = self->TheModule->getFunction ("sqrt");
    assert (call_f);
    llvm::Value *arg = Create_expression (self, node->left);
    llvm::Value *sqrt = self->builder->CreateCall (call_f, arg);
    return sqrt;
}


static llvm::Value *Create_input (Creator_t *self, Tree_node *node)
{
    llvm::Function *call_f = self->TheModule->getFunction ("input");

    llvm::Value *input = self->builder->CreateCall (call_f);
    return input;
}



llvm::Value *Create_calling (Creator_t *self, Tree_node *node)
{
    llvm::Function *call_f = self->TheModule->getFunction (node->left->value.id);

    Tree_node *param_node = node->left->left;

    std::vector<llvm::Value *> params (0); 
    while (param_node)
    {
        llvm::Value *param = Create_expression (self, param_node->left);
        params.push_back (param);
        param_node = param_node->right;
    }
    
    llvm::Value *call = self->builder->CreateCall (call_f, params);
    return call;
}


void Create_return (Creator_t *self, Tree_node *node)
{
    llvm::Value *ret_value = Create_expression (self, node->left);
    self->builder->CreateRet (ret_value);
}


void Create_function_proto (Creator_t *self, Tree_node *node)
{
    size_t params_size = 0;
    Tree_node *params = node->left->left;

    self->names = (Names_t *) calloc (1, sizeof (Names_t));
    self->names->number = 0;
    self->names->variables = (Var *) calloc (1, sizeof (Var));

    while (params)
    {
        params_size += 1;
        params = params->right;
    }

    std::vector<llvm::Type *> args (params_size, self->type);
    puts ("i am in function");
    llvm::FunctionType *func_t = llvm::FunctionType::get(self->type, args, false);
    llvm::Function *func = llvm::Function::Create(func_t, llvm::Function::ExternalLinkage, node->left->value.id, *self->TheModule);
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(*self->Context, "entry", func);

    self->cur_func = func;
    self->builder->SetInsertPoint(entry);

    params = node->left->left;

    for (size_t index = 0; index < params_size; index ++)
    {
        llvm::Value *var = func->getArg (index);
        var->setName (params->left->left->value.id);
        llvm::Value *alloca = self->builder->CreateAlloca (self->type);
        self->builder->CreateStore (var, alloca);
        
        self->names->variables[self->names->number].name = params->left->left->value.id;
        self->names->variables[self->names->number].node = alloca;
        self->names->number += 1;
        self->names->variables = (Var *) realloc (self->names->variables, sizeof (Var) * (self->names->number + 1));

        params = params->right;
    }
    Create_statement (self, node->right);
    free (self->names->variables);
    free (self->names);
}


void Create_assignment (Creator_t *self, Tree_node *node)
{
    llvm::Value *var = Find_var (self->names, node->left->value.id);
    llvm::Value *result = Create_expression (self, node->right);
    self->builder->CreateStore (result, var);
}


static inline llvm::Value *Create_bin_operator (Creator_t *self, Tree_node *node)
{
    llvm::Value *left = Create_expression  (self, node->left);
    llvm::Value *right = Create_expression (self, node->right);

    switch (node->value.op)
    {
    case OP_ADD:
        return self->builder->CreateAdd (left, right);
    case OP_SUB:
        return self->builder->CreateSub (left, right);
    case OP_MUL:
        return self->builder->CreateMul (left, right);
    case OP_DIV:
        return self->builder->CreateSDiv (left, right);
    default:
        break;
    }
    return nullptr;
}




llvm::Value *Create_expression (Creator_t *self, Tree_node *node)
{
    if (node->type == OPERATOR)
    {
        switch (node->value.op)
        {
        default:
            puts ("unknown operator");
            exit (-1);
        case OP_CALL:
            return Create_calling (self, node);
        case OP_INPUT:
            return Create_input (self, node);
        case OP_SQRT:
            return Create_sqrt (self, node);
        case OP_MUL: case OP_SUB:
        case OP_DIV: case OP_ADD:
            return Create_bin_operator (self, node);
        }
    }
    else if (node->type == IDENTIFIER)
    {
        llvm::Value *var = Find_var (self->names, node->value.id);
        return self->builder->CreateLoad (self->type, var);
    }
    else if (node->type == NUMBER)
        return self->builder->getInt64 (node->value.num * 100);

    return nullptr;
}


llvm::Value * Create_condition (Creator_t *self, Tree_node *node)
{
    llvm::Value *left = Create_expression (self, node->left);
    llvm::Value *right = Create_expression (self, node->right);

    switch (node->value.op)
    {
    case OP_IS_GT:
        return self->builder->CreateICmpSGT (left, right);
    case OP_IS_BT:
        return self->builder->CreateICmpSLT (left, right);
    case OP_IS_NE:
        return self->builder->CreateICmpNE (left, right);
    case OP_IS_EE:
        return self->builder->CreateICmpEQ (left, right);
    default:
        break;
    }
    return nullptr;
}


// if (cond) then (body) else (body) next ()
// if (cond) then (body) next (body)
void Create_comparsion (Creator_t *self, Tree_node *node)
{
    llvm::BasicBlock *if_block =llvm::BasicBlock::Create (*self->Context, "if", self->cur_func);
    self->builder->CreateBr (if_block);
    self->builder->SetInsertPoint (if_block);
    llvm::Value *res = Create_condition (self, node->left);

    if (node->right->value.op != OP_ELSE)
    {
        llvm::BasicBlock *then = llvm::BasicBlock::Create (*self->Context, "then", self->cur_func); 
        llvm::BasicBlock *next = llvm::BasicBlock::Create (*self->Context, "next", self->cur_func);
        llvm::Value *br  = self->builder->CreateCondBr (res, then, next);

        self->builder->SetInsertPoint (then);
        Create_statement (self, node->right);
        self->builder->CreateBr (next);
        self->builder->SetInsertPoint (next);
        self->lastBB = next;
    }
    else
    {
        Tree_node *else_node = node->right;

        llvm::BasicBlock *then = llvm::BasicBlock::Create (*self->Context, "then", self->cur_func); 
        llvm::BasicBlock *else_block = llvm::BasicBlock::Create (*self->Context, "else", self->cur_func);
        llvm::BasicBlock *next = llvm::BasicBlock::Create (*self->Context, "next", self->cur_func);
        self->builder->CreateCondBr (res, then, else_block);

        self->builder->SetInsertPoint (then);
        Create_statement (self, else_node->left);
        self->builder->CreateBr (next);


        self->builder->SetInsertPoint (else_block);
        self->lastBB = else_block;
        Create_statement (self, else_node->right);
        self->builder->CreateBr (next);

        self->builder->SetInsertPoint (next);
        self->lastBB = next;
        printf ("else...\n");
    }

}


void Create_var (Creator_t *self, Tree_node *node)
{
    assert (node);

    llvm::Value *x   = self->builder->CreateAlloca (self->type);
    llvm::Value *num = Create_expression (self, node->right);
    llvm::Value *store = self->builder->CreateStore (num, x);

    self->names->variables[self->names->number].name = node->left->value.id;
    self->names->variables[self->names->number].node = x;
    self->names->number += 1;
    self->names->variables = (Var *) realloc (self->names->variables, (self->names->number + 1)* sizeof (Var));
}


void Create_statement (Creator_t *self, Tree_node *node)
{
    if (!node) return;
    
    if (node->type == OPERATOR)
    {
        switch (node->value.op)
        {
            case OP_ST:
                Create_statement (self, node->left);
                Create_statement (self, node->right);
                break;
            case OP_VAR:
                Create_var (self, node);
                break;
            case OP_IF:
                Create_comparsion (self, node);
                break;
            case OP_ADD_ASSIGN: case OP_SUB_ASSIGN:
            case OP_ASSGN:
                Create_assignment (self, node);
                break;
            case OP_RETURN:
                Create_return (self, node);
                break;
            case OP_PRINT:
                Create_print (self, node);
                break;
            default:
                break;
        }
    }
}


void Create_IR (Root *tree_root, const char *ir_file_name)
{
    assert (tree_root);
    assert (ir_file_name);

    Creator_t *Creator = (Creator_t *) calloc (1, sizeof (Creator_t));
    assert (Creator);

    Creator->Context   = new llvm::LLVMContext;
    Creator->builder   = new llvm::IRBuilder<>(*Creator->Context);
    Creator->TheModule = new llvm::Module ("Creator", *Creator->Context);

    llvm::Type *var_type =  llvm::Type::getInt64Ty (*Creator->Context);
    Creator->type = var_type;


    Tree_node *statement = tree_root->first_node;

    std::vector<llvm::Type *> args (1, Creator->type);
    llvm::FunctionType *print_t = llvm::FunctionType::get (llvm::Type::getVoidTy (*Creator->Context), Creator->type, false);
    llvm::Function *print = llvm::Function::Create(print_t, llvm::Function::ExternalLinkage, "print", *Creator->TheModule);

    llvm::FunctionType *input_t = llvm::FunctionType::get (Creator->type, false);
    llvm::Function::Create(input_t, llvm::Function::ExternalLinkage, "input", *Creator->TheModule);

    llvm::FunctionType *sqrt_t = llvm::FunctionType::get (Creator->type, Creator->type, false);
    llvm::Function::Create(sqrt_t, llvm::Function::ExternalLinkage, "sqrt", *Creator->TheModule);


    while (statement)
    {
        if (statement->left->value.op != OP_FUNC)
        {
            puts ("NOT A FUNCTION!!!");
            return;
        }
        Create_function_proto (Creator, statement->left);
        statement = statement->right;
    }

    llvm::StringRef bc_file_name = llvm::StringRef (ir_file_name);
    std::error_code error_code  = std::error_code ();
    llvm::raw_fd_ostream bc_file (bc_file_name, error_code);
    WriteBitcodeToFile (*Creator->TheModule,  bc_file);


    llvm::StringRef dump_file_name = llvm::StringRef ("Creator.ll");
    llvm::raw_fd_ostream my_file (dump_file_name, error_code);

    Creator->TheModule->print (my_file, nullptr);

    delete Creator->TheModule;
    delete Creator->builder;
    delete Creator->Context;
    free (Creator);

}
