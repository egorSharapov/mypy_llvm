#pragma once 

#include <llvm-c/Core.h>
#include "../include/ir2asm.hpp"
#include "../include/emit.hpp"


void FillFunctionCallRegs     (Callee *callee_regs, LLVMValueRef func, const Regs *calling_reg);
LLVMModuleRef ParseModule     (const char *bitcode_file_name);
void CompileLoad              (FILE *out_file, LLVMValueRef instruction, Executor_t *executor, Execute_module *self);
void CompileStore             (FILE *out_file, LLVMValueRef instruction, Executor_t *executor, Execute_module *self);
void CompileMul               (FILE *out_file, LLVMValueRef instruction, LLVMValueRef x, LLVMValueRef y, Executor_t *executor, Execute_module *self);
void CompileDiv               (FILE *out_file, LLVMValueRef instruction, LLVMValueRef x, LLVMValueRef y, Executor_t *executor, Execute_module *self);
void CompileAdd               (FILE *out_file, LLVMValueRef instruction, LLVMValueRef x, LLVMValueRef y, Executor_t *executor, Execute_module *self);
void CompileSub               (FILE *out_file, LLVMValueRef instruction, LLVMValueRef x, LLVMValueRef y, Executor_t *executor, Execute_module *self);
void CompileRet               (FILE *out_file, LLVMValueRef instruction, uint32_t start_label, LLVMValueRef func, Executor_t *executor, Execute_module *self);
void CompileBr                (FILE *out_file, LLVMValueRef instruction, LLVMIntPredicate predicate, LLVMBasicBlockRef basic_block, Executor_t *executor, Execute_module *self);
void CompileCmp               (FILE *out_file, LLVMValueRef instruction, LLVMIntPredicate *predicate, Executor_t *executor, Execute_module *self);
void CompileCall              (FILE *out_file, LLVMValueRef instruction, const Regs *calling_reg, const int VAR_SIZE, Executor_t *executor, Execute_module *self);
void CompileBinaryInstruction (FILE *out_file, LLVMValueRef instruction, Executor_t *executor, Execute_module *self);
void Compile_func_args        (FILE *out_file, LLVMValueRef instruction, const Regs *calling_reg, Executor_t *executor, Execute_module *self);