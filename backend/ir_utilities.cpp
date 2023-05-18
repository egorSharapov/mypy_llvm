#include "../include/ir_utilities.hpp"
#include "../include/ir2asm.hpp"

#ifndef NDEBUG
#define DUMP(...) fprintf(out_file, __VA_ARGS__)
#else
#define DUMP(...) ;
#endif

void FillFunctionCallRegs (Callee *callee_regs, LLVMValueRef func, const Regs *calling_reg)
{
    size_t index = 0;
    LLVMValueRef *params = (LLVMValueRef *) calloc (LLVMCountParams (func), sizeof (LLVMValueRef));
    LLVMGetParams (func, params);
    if (params)
    {
        while (params[index])
        {
            callee_regs->array[callee_regs->size].node = params[index];
            callee_regs->array[callee_regs->size].reg = calling_reg[callee_regs->size];
            callee_regs->size += 1;
            index += 1;
        }
    }
}


LLVMModuleRef ParseModule (const char *bitcode_file_name)
{

    LLVMMemoryBufferRef memoryBuffer = nullptr;
    char *message = nullptr;
    if (0 != LLVMCreateMemoryBufferWithContentsOfFile(bitcode_file_name, &memoryBuffer, &message)) 
    {
        fprintf(stderr, "|%s|\n", message);
        free(message);
        return nullptr;
    }

    LLVMModuleRef module = nullptr;
    if (0 != LLVMParseBitcode2(memoryBuffer, &module)) 
    {
        fprintf(stderr, "Invalid bitcode detected!\n");
        puts ("invalid bitcode");
        LLVMDisposeMemoryBuffer(memoryBuffer);
        return nullptr;
    }
    puts ("succes parse");
    LLVMDisposeMemoryBuffer(memoryBuffer);

    return module;
}



void CompileLoad (FILE *out_file, LLVMValueRef instruction, Executor_t *executor, Execute_module *self)
{
    LLVMValueRef transmitter = LLVMGetOperand (instruction, 0);
    size_t local_var_index = FindLocalVar (executor->locals, transmitter);

    FixReg (executor->fixup, instruction);
    DUMP ("    mov %%%d, [r8+%ld]; Emmited\n", FindReg (executor->fixup, instruction), local_var_index);
    Emit_mov_r_m (self, FindReg (executor->fixup, instruction), local_var_index);
}



void CompileStore (FILE *out_file, LLVMValueRef instruction, Executor_t *executor, Execute_module *self)
{
    LLVMValueRef source = LLVMGetOperand (instruction, 0);
    LLVMValueRef transmitter = LLVMGetOperand (instruction, 1);
    size_t local_var_index = FindLocalVar (executor->locals, transmitter);

    if (!LLVMIsConstant (source))
    {
        Regs reg = FindParam (executor->callee_regs, source);
        if (reg == none)
        {
            reg = FindReg (executor->fixup, source);
            FreeReg (executor->fixup, source);
        }
        DUMP ("    mov [r8+%ld], %%%d; Emmited\n", local_var_index, reg);
        Emit_mov (self, local_var_index, reg);
    }
    else
    {
        DUMP ("    mov [r8+%ld], %lld; Emmited\n", local_var_index, LLVMConstIntGetSExtValue (source));
        Emit_mov_m_i (self, local_var_index, LLVMConstIntGetSExtValue (source));
    }
}



void CompileMul (FILE *out_file, LLVMValueRef instruction, LLVMValueRef x, LLVMValueRef y, Executor_t *executor, Execute_module *self)
{
    if (!LLVMIsConstant (x) and !LLVMIsConstant (y))
    {
        DUMP ("    cvtsi2ss xmm1, %%%d\n"
              "    cvtsi2ss xmm2, %%%d\n"
              "    mulss xmm1, xmm2\n"
              "    divss xmm1, xmm5\n"
              "    cvtss2si %%%d, xmm1\n", FindReg (executor->fixup, x), FindReg (executor->fixup, y), FindReg (executor->fixup, x));

        Emit_cvtsi2ss (self, xmm1, FindReg (executor->fixup, x));
        Emit_cvtsi2ss (self, xmm2, FindReg (executor->fixup, y));

        Emit_mulss (self, xmm1, xmm2);
        Emit_divss (self, xmm1, xmm5);

        Emit_cvtss2si (self, FindReg (executor->fixup, x), xmm1);
                                
        FreeReg (executor->fixup, x);
        FreeReg (executor->fixup, y);
        FixReg (executor->fixup, instruction);
    }
    else if (LLVMIsConstant (x))
    {
        DUMP ("    mov rax, %lld\n"
              "    cvtsi2ss xmm1, rax\n"
              "    cvtsi2ss xmm2, %%%d\n"
              "    mulss xmm1, xmm2\n"
              "    divss xmm1, xmm5\n"
              "    cvtss2si %%%d, xmm1\n", LLVMConstIntGetSExtValue (x), 
                                           FindReg (executor->fixup, y), FindReg (executor->fixup, y));

        Emit_mov (self, rax, (uint32_t) LLVMConstIntGetSExtValue (x));
        Emit_cvtsi2ss (self, xmm1, rax);
        Emit_cvtsi2ss (self, xmm2, FindReg (executor->fixup, y));

        Emit_mulss (self, xmm1, xmm2);
        Emit_divss (self, xmm1, xmm5);

        Emit_cvtss2si (self, FindReg (executor->fixup, y), xmm1);
        FreeReg (executor->fixup, y);
        FixReg (executor->fixup, instruction);  
    }
    else
        DUMP ("    mul...\n");
}


void CompileDiv (FILE *out_file, LLVMValueRef instruction, LLVMValueRef x, LLVMValueRef y, Executor_t *executor, Execute_module *self)
{
    if (LLVMIsConstant (y))
    {
        DUMP ("    mov rax, %lld\n"
              "    cvtsi2ss xmm2, rax\n"
              "    cvtsi2ss xmm1, %%%d\n"
              "    mulss xmm1, xmm5\n"
              "    divss xmm1, xmm2\n"
              "    cvtss2si %%%d, xmm1\n", LLVMConstIntGetSExtValue (y), 
                                           FindReg (executor->fixup, x),
                                           FindReg (executor->fixup, x));
                            
        Emit_mov (self, rax, (uint32_t) LLVMConstIntGetSExtValue (y), false);
        Emit_cvtsi2ss (self, xmm2, rax);
        Emit_cvtsi2ss (self, xmm1, FindReg (executor->fixup, x));
        Emit_mulss (self, xmm1, xmm5);
        Emit_divss (self, xmm1, xmm2);
        Emit_cvtss2si (self, FindReg (executor->fixup, x), xmm1);
        FreeReg (executor->fixup, x);
        FixReg (executor->fixup, instruction);
    }
    else if (LLVMIsConstant (y))
    {
        DUMP ("    idiv...\n");
    }
    else 
        DUMP ("    cvtsi2ss xmm2, %%%d\n"
              "    cvtsi2ss xmm1, %%%d\n"
              "    mulss xmm1, xmm5\n"
              "    divss xmm1, xmm2\n"
              "    cvtss2si %%%d, xmm1\n", FindReg (executor->fixup, y), 
                                           FindReg (executor->fixup, x),
                                           FindReg (executor->fixup, x));
                            
    Emit_cvtsi2ss (self, xmm2, FindReg (executor->fixup, y));
    Emit_cvtsi2ss (self, xmm1, FindReg (executor->fixup, x));
    Emit_mulss (self, xmm1, xmm5);
    Emit_divss (self, xmm1, xmm2);
    Emit_cvtss2si (self, FindReg (executor->fixup, x), xmm1);
                            
    FreeReg (executor->fixup, x);
    FreeReg (executor->fixup, y);
    FixReg (executor->fixup, instruction);
}


void CompileAdd (FILE *out_file, LLVMValueRef instruction, LLVMValueRef x, LLVMValueRef y, Executor_t *executor, Execute_module *self)
{
    if (!LLVMIsConstant (x) and !LLVMIsConstant (y))
    {
        DUMP ("    add %%%d, %%%d; Emmited\n", FindReg (executor->fixup, x), FindReg (executor->fixup, y));
        Emit_add_r_r (self, FindReg (executor->fixup, x), FindReg (executor->fixup, y));

        FreeReg (executor->fixup, x);
        FreeReg (executor->fixup, y);
        
    }
    else if (!LLVMIsConstant (x))
    {
        DUMP ("    add %%%d, %lld; Emmited 2\n", FindReg (executor->fixup, x), LLVMConstIntGetSExtValue (y));
        Emit_add (self, FindReg (executor->fixup, x), LLVMConstIntGetSExtValue (y), true);

        FreeReg (executor->fixup, x);
    }
    else 
    {
        DUMP ("    add %%%d, %lld; Emmited 1\n", FindReg (executor->fixup, y), LLVMConstIntGetSExtValue (x));
        Emit_add (self, FindReg (executor->fixup, y), LLVMConstIntGetSExtValue (x), false);
        FreeReg (executor->fixup, y);
    }
    FixReg (executor->fixup, instruction);
}


void CompileSub (FILE *out_file, LLVMValueRef instruction, LLVMValueRef x, LLVMValueRef y, Executor_t *executor, Execute_module *self)
{
    if (LLVMIsConstant (x))
    {
        DUMP ("    sub %%%d, %lld\n", FindReg (executor->fixup, y), LLVMConstIntGetSExtValue (x));
        DUMP ("    neg reg\n");
        
        Emit_sub (self, FindReg (executor->fixup, y), LLVMConstIntGetSExtValue (x), false);
        Emit_neg (self, FindReg (executor->fixup, y));
        FreeReg (executor->fixup, y);
        FixReg (executor->fixup, instruction);
    }
    else if (LLVMIsConstant (y))
    {
        DUMP ("    sub %%%d, %lld\n", FindReg (executor->fixup, y), LLVMConstIntGetSExtValue (x));

        Emit_sub (self, FindReg (executor->fixup, y), LLVMConstIntGetSExtValue (x), false);

        FreeReg (executor->fixup, y);
        FixReg (executor->fixup, instruction);    
    }
    else
    {
        DUMP ("    sub %%%d, %%%d\n", FindReg (executor->fixup, x), FindReg (executor->fixup, y));

        Emit_sub (self, FindReg (executor->fixup, x), FindReg (executor->fixup, y), false);
        FreeReg (executor->fixup, x);
        FreeReg (executor->fixup, y);
        FixReg (executor->fixup, instruction);
    }
}


void CompileRet (FILE *out_file, LLVMValueRef instruction, uint32_t start_label, LLVMValueRef func, Executor_t *executor, Execute_module *self)
{
    LLVMValueRef ret_value = LLVMGetOperand (instruction, 0);
    
    if (LLVMIsConstant (ret_value))
    {
        DUMP ("    mov rax, %lld; Emmited\n", LLVMConstIntGetSExtValue (ret_value));
        Emit_mov (self, rax, (uint32_t) LLVMConstIntGetSExtValue (ret_value));
    }
    else
    {
        DUMP ("    mov rax, %%%d; Emmited\n", FindReg (executor->fixup, ret_value));
        Emit_mov (self, rax, FindReg (executor->fixup, ret_value), false);
    }
    if (GetFunctionLabel (executor->table, func) != start_label)
    {
        DUMP ("    ret; Emmited\n");
        Emit_ret (self);
    }
    else 
    {
        DUMP ("    exit (0)\n");
        Emit_mov (self, rax, (uint32_t) 60, false);
        Emit_xor (self, rdi, rdi);
        Emit_syscall (self);
    }
}


void CompileBr (FILE *out_file, LLVMValueRef instruction, LLVMIntPredicate predicate, LLVMBasicBlockRef basic_block, Executor_t *executor, Execute_module *self)
{
    if (LLVMGetNumOperands (instruction) == 1)
    {
        LLVMBasicBlockRef jmp_point = LLVMGetSuccessor (instruction, 0);
        if (LLVMGetNextBasicBlock(basic_block) != jmp_point)
        {
            DUMP ("    jmp %s (%lu)\n", LLVMGetBasicBlockName (jmp_point), GetBasicBlockLabel (executor->table, jmp_point));
            Emit_jumps (self, JMP, GetBasicBlockLabel (executor->table, jmp_point), false);
        }
    }
    else
    {
        Instructions jcc = JNE; 
        Instructions jnc = JE; 
        switch (predicate)
        {
            case LLVMIntSLT:
            case LLVMIntSLE:
                jcc = JGE; 
                jnc = JLE;
                break;
            case LLVMIntSGT:
            case LLVMIntSGE:
                jcc = JLE; 
                jnc = JGE;
                break;
            case LLVMIntNE:
                jcc = JE; 
                jnc = JNE;
                break;
            case LLVMIntEQ:
                jcc = JNE;  
                jnc = JE;
                break;
            default:
                break;
        }   
        LLVMBasicBlockRef true_cond = LLVMGetSuccessor (instruction, 1);
        DUMP ("    jcc %s (%lu); Emmited\n", LLVMGetBasicBlockName (true_cond), GetBasicBlockLabel (executor->table, true_cond));
        Emit_jumps (self, jcc, GetBasicBlockLabel (executor->table, true_cond), true);

        LLVMBasicBlockRef false_cond = LLVMGetSuccessor (instruction, 0);              
        DUMP ("    jnc %s (%lu); Emmited\n", LLVMGetBasicBlockName (false_cond), GetBasicBlockLabel (executor->table, false_cond));
        Emit_jumps (self, jnc, GetBasicBlockLabel (executor->table, false_cond), true);
    }
}


void CompileCmp (FILE *out_file, LLVMValueRef instruction, LLVMIntPredicate *predicate, Executor_t *executor, Execute_module *self)
{
    LLVMValueRef first = LLVMGetOperand (instruction, 0);
    LLVMValueRef second = LLVMGetOperand (instruction, 1);
    
    *predicate = LLVMGetICmpPredicate (instruction);

    if (!LLVMIsConstant (first) and !LLVMIsConstant (second)) 
    {
        DUMP ("    cmp %%%d, %%%d; Emmited\n", FindReg (executor->fixup, first), FindReg (executor->fixup, second));
        Emit_cmp (self, FindReg (executor->fixup, first), FindReg (executor->fixup, second));
    }
    else
    {
        DUMP ("    cmp %%%d, %lld; Emmited\n", FindReg (executor->fixup, first), LLVMConstIntGetSExtValue (second));
        Emit_cmp (self, FindReg (executor->fixup, first), LLVMConstIntGetSExtValue (second)); 
    }
}



void Compile_func_args (FILE *out_file, LLVMValueRef instruction, const Regs *calling_reg, Executor_t *executor, Execute_module *self)
{
    unsigned calling_reg_number = LLVMGetNumArgOperands (instruction);
                    
    for (unsigned index = 0; index < calling_reg_number; index++)
    {
        LLVMValueRef arg = LLVMGetArgOperand (instruction, index);
        if (LLVMIsConstant(arg))
        {
            DUMP ("    mov %%%d, %lld; Emmited\n", calling_reg[index], LLVMConstIntGetSExtValue (arg));
            Emit_mov (self, calling_reg[index], (uint32_t) LLVMConstIntGetSExtValue (arg), false);
        }
        else
        {
            DUMP ("    mov %%%d, %%%d; Emmited\n", calling_reg[index], FindReg (executor->fixup, arg));
            Emit_mov (self, calling_reg[index], FindReg (executor->fixup, arg), false);
            FreeReg (executor->fixup, arg);
        }
    }
    for (int index = 0; index < MAX_REG_IN_USE; index++)
    {
        if (executor->fixup->array[index].node != nullptr)
        {
            DUMP ("    push %%%d\n", executor->fixup->array[index].reg);
            Emit_push (self, executor->fixup->array[index].reg, false);
        }
    }   
}


