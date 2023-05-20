#pragma once

#include <stdio.h>
#include <malloc.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/Core.h>
#include "../../JIT/JIT.hpp"
#include "emit.hpp"


#define MAX_REG_IN_USE 6


typedef struct 
{
    Regs reg;
    LLVMValueRef node;
} Callee_node;


typedef struct
{
    Callee_node *array;
    size_t size;
} Callee;


typedef struct 
{
    size_t index;
    LLVMValueRef node;
} Locals_node;


typedef struct
{
    Locals_node *array;
    size_t size;
} Locals;


typedef struct
{
    size_t label;
    LLVMBasicBlockRef block;
    LLVMValueRef      func;
} Labels_node;


typedef struct 
{
    Labels_node *array;
    size_t size;
} Labels_table;


typedef struct 
{
    Regs reg;
    LLVMValueRef node;
} Reg_fix;


typedef struct 
{
    Reg_fix array[MAX_REG_IN_USE];
    size_t size;
} Fixup_t;

typedef struct 
{
    Labels_table *table;
    Callee *callee_regs;
    Locals *locals;
    Fixup_t *fixup;
} Executor_t;


Execute_module *Create_asm (const char *bitcode_file_name, const char *out_file_name);
size_t GetFunctionLabel (Labels_table *table, LLVMValueRef func);
size_t GetBasicBlockLabel (Labels_table *table, LLVMBasicBlockRef value);
Regs FindParam (Callee *call_reg, LLVMValueRef value);
size_t FindLocalVar (Locals *locals, LLVMValueRef value);
void FixReg (Fixup_t *fixup, LLVMValueRef value);
void FreeReg (Fixup_t *fixup, LLVMValueRef value);
Regs FindReg (Fixup_t *fixup, LLVMValueRef value);
void ReFixReg (Fixup_t *fixup, LLVMValueRef source, LLVMValueRef dest);
