#pragma once
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IRReader/IRReader.h"
#include <llvm-c/BitReader.h>
#include <llvm-c/BitWriter.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include "../lex_tree/tree.hpp"


typedef struct 
{
    const char *name;
    llvm::Value *node;
} Var;


typedef struct 
{
    const char *name;
    llvm::Function *node;
} Function_t;


typedef struct 
{
    Function_t *func;
    size_t number;
} FNames_t;


typedef struct 
{
    Var *variables;
    size_t number;
} Names_t;


typedef struct 
{
    Names_t *names;
    FNames_t *functions;
    llvm::LLVMContext *Context;
    llvm::IRBuilder<> *builder;
    llvm::Module *TheModule;
    llvm::Function *cur_func;
    llvm::BasicBlock *lastBB;
    llvm::Type *type;
} Creator_t;


void  Create_IR             (Root *tree_root, const char *ir_file_name);
void  Create_function_proto (Creator_t *self, Tree_node *node);
void  Create_comparsion     (Creator_t *self, Tree_node *node);
void  Create_assignment     (Creator_t *self, Tree_node *node);
llvm::Value *Create_expression    (Creator_t *self, Tree_node *node);
llvm::Value * Create_condition    (Creator_t *self, Tree_node *node);
void  Create_statement      (Creator_t *self, Tree_node *node);
void  Create_return         (Creator_t *self, Tree_node *node);
llvm::Value *Create_calling       (Creator_t *self, Tree_node *node);

