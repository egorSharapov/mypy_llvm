#include "../include/ir2asm.hpp"
#include "../../JIT/JIT.hpp"
#include "../include/emit.hpp"
#include "../include/ir_utilities.hpp"

const int      VAR_SIZE = 8;
const int      MAX_LOCAL_VARS_NUMBER = 10;
const int      MAX_CALL_REGS_NUMBER  = 4;
const int      MAX_BUFFER_SIZE       = 4096;
const uint32_t FIXED_PRECISION_MULTIPLIER = 100;
const uint64_t RELATIVE_COUT_ADDRES = 0x3000;
const uint64_t RELATIVE_CIN_ADDRES  = 0x3158;
const uint64_t RELATIVE_SQRT_ADDRES = 0x326d;
const uint64_t MEMORY_BUFFER_ADDRES = 0x402000;

const Regs calling_reg[MAX_CALL_REGS_NUMBER] = {rdi, rsi, rdx, rcx};
const Regs reg_in_use[MAX_REG_IN_USE] = {rbx, rcx, rdx, rsi, rbp, rdi};
static int labels_number = 0;
LLVMIntPredicate predicate = LLVMIntEQ;


#ifndef NDEBUG
void DumpFixup(Fixup_t *fixup)
{
    for (int index = 0; index < MAX_REG_IN_USE; index++)
    {
        printf(" %p |", fixup->array[index].node);
    }
    puts("");
    for (int index = 0; index < MAX_REG_IN_USE; index++)
    {
        printf("        %d       |", fixup->array[index].reg);
    }
    puts("");
}
#endif


Regs FindReg(Fixup_t *fixup, LLVMValueRef value)
{
    for (int index = 0; index < MAX_REG_IN_USE; index++)
    {
        if (fixup->array[index].node == value)
            return fixup->array[index].reg;
    }
    return none;
}

void FreeReg(Fixup_t *fixup, LLVMValueRef value)
{
    for (int index = 0; index < MAX_REG_IN_USE; index++)
    {
        if (fixup->array[index].node == value)
        {
            fixup->array[index].node = nullptr;
            return;
        }
    }
}


void FixReg(Fixup_t *fixup, LLVMValueRef value)
{
    #ifndef NDEBUG
    DumpFixup(fixup);
    #endif

    for (int index = 0; index < MAX_REG_IN_USE; index++)
    {
        if (fixup->array[index].node == nullptr)
        {
            fixup->array[index].node = value;
            return;
        }
    }
    #ifndef NDEBUG
    DumpFixup(fixup);
    #endif
}


static void FreeFixup (Fixup_t *fixup)
{
    for (int index = 0; index < MAX_REG_IN_USE; index++)
    {
        fixup->array[index].node = NULL;
    }
    puts("fixup cleaned!");
}

size_t FindLocalVar (Locals *locals, LLVMValueRef value)
{
    for (int index = 0; index < sizeof(calling_reg); index++)
    {
        if (locals->array[index].node == value)
            return locals->array[index].index * VAR_SIZE;
    }
    return -1;
}

Regs FindParam (Callee *call_reg, LLVMValueRef value)
{
    for (int index = 0; index < call_reg->size; index++)
    {
        if (call_reg->array[index].node == value)
            return call_reg->array[index].reg;
    }
    return none;
}

size_t GetBasicBlockLabel (Labels_table *table, LLVMBasicBlockRef value)
{
    for (int index = 0; index < table->size; index++)
    {
        if (table->array[index].block == value)
            return table->array[index].label;
    }
    return -1;
}

size_t GetFunctionLabel(Labels_table *table, LLVMValueRef func)
{
    for (int index = 0; index < table->size; index++)
    {
        if (table->array[index].func == func)
            return table->array[index].label;
    }
    return -1;
}

static void GenLabel(Labels_table *table, LLVMValueRef func, LLVMBasicBlockRef basic_block, int labels_number)
{
    table->array[table->size].func = func;
    table->array[table->size].block = basic_block;
    table->array[table->size].label = labels_number++;
    table->size += 1;
}

static Labels_table *CreateLabelsTable(LLVMModuleRef module, uint8_t *main_label)
{
    Labels_table *table = (Labels_table *)calloc(1, sizeof(Labels_table));
    LLVMValueRef main = LLVMGetNamedFunction(module, "main");

    table->array = (Labels_node *)calloc(100, sizeof(Labels_node));
    table->size = 0;
    for (LLVMValueRef func = LLVMGetFirstFunction(module); func; func = LLVMGetNextFunction(func))
    {
        if (func == main)
            *main_label = labels_number;
        GenLabel(table, func, nullptr, labels_number++);

        for (LLVMBasicBlockRef basic_block = LLVMGetFirstBasicBlock(func);
             basic_block; basic_block = LLVMGetNextBasicBlock(basic_block))
            GenLabel(table, nullptr, basic_block, labels_number++);
    }
    return table;
}


static Executor_t *Executor_init (int max_reg_in_use, int call_reg_size, const int max_local_vars_size)
{
    Executor_t *executor = (Executor_t *) calloc (1, sizeof (Executor_t));

    executor->fixup = (Fixup_t *)calloc(1, sizeof(Fixup_t));
    for (int index = 0; index < MAX_REG_IN_USE; index++)
    {
        executor->fixup->array[index].reg = reg_in_use[index];
        executor->fixup->array[index].node = nullptr;
    }
    executor->fixup->size = max_reg_in_use;

    executor->callee_regs = (Callee *) calloc (1, sizeof(Callee));
    executor->callee_regs->array = (Callee_node *) calloc (call_reg_size, sizeof(Callee_node));
    executor->callee_regs->size = 0;

    executor->locals = (Locals *)calloc(1, sizeof(Locals));
    executor->locals->array = (Locals_node *) calloc (max_local_vars_size, sizeof(Locals_node));
    executor->locals->size = 0;

    return executor;
}

#define DUMP(...) fprintf(out_file, __VA_ARGS__)

Execute_module *Create_asm (const char *bitcode_file_name, const char *out_file_name)
{
    FILE *out_file = fopen(out_file_name, "w");
    if (!out_file)
    {
        fprintf(stderr, "cant open out file\n");
        return nullptr;
    }
    puts("succes open dump file");

    Executor_t *executor = Executor_init (MAX_REG_IN_USE, sizeof (calling_reg), MAX_LOCAL_VARS_NUMBER);
    LLVMModuleRef module = ParseModule (bitcode_file_name);

    uint8_t start_label;
    executor->table = CreateLabelsTable (module, &start_label);
    Execute_module *self = Execute_module_ctor (MAX_BUFFER_SIZE);

    Create_label(self, 0x0, RELATIVE_COUT_ADDRES);
    Create_label(self, 0x1, RELATIVE_CIN_ADDRES);
    Create_label(self, 0x2, RELATIVE_SQRT_ADDRES);

    DUMP("    mov rax, 100\n");
    DUMP("    cvtsi2ss xmm5, rax\n");

    Emit_mov(self, rax, FIXED_PRECISION_MULTIPLIER, false);
    Emit_cvtsi2ss(self, xmm5, rax);

    DUMP("    movabs r8, memory address\n");
    Emit_mov(self, r8, MEMORY_BUFFER_ADDRES, true);
    Emit_jumps(self, JMP, start_label, false);

    for (LLVMValueRef func = LLVMGetFirstFunction(module); func; func = LLVMGetNextFunction(func))
    {
        DUMP("@%s (%lu):\n", LLVMGetValueName(func), GetFunctionLabel(executor->table, func));
        if (GetFunctionLabel(executor->table, func) > 2)
            Create_label(self, GetFunctionLabel(executor->table, func), self->rip);

        FillFunctionCallRegs(executor->callee_regs, func, calling_reg);

        for (LLVMBasicBlockRef basic_block = LLVMGetFirstBasicBlock(func);
             basic_block; basic_block = LLVMGetNextBasicBlock(basic_block))
        {
            DUMP("  %s (%lu):\n", LLVMGetBasicBlockName(basic_block), GetBasicBlockLabel(executor->table, basic_block));
            Create_label(self, GetBasicBlockLabel(executor->table, basic_block), self->rip);

            for (LLVMValueRef instruction = LLVMGetFirstInstruction(basic_block); instruction; instruction = LLVMGetNextInstruction(instruction))
            {
                if (LLVMIsABinaryOperator(instruction))
                {
                    LLVMValueRef x = LLVMGetOperand(instruction, 0);
                    LLVMValueRef y = LLVMGetOperand(instruction, 1);

                    switch (LLVMGetInstructionOpcode(instruction))
                    {
                    case LLVMAdd:
                        CompileAdd(out_file, instruction, x, y, executor, self);
                        break;
                    case LLVMSub:
                        CompileSub(out_file, instruction, x, y, executor, self);
                        break;
                    case LLVMMul:
                        CompileMul(out_file, instruction, x, y, executor, self);
                        break;
                    case LLVMSDiv:
                        CompileDiv(out_file, instruction, x, y, executor, self);
                        break;
                    default:
                        break;
                    }
                }
                switch (LLVMGetInstructionOpcode(instruction))
                {
                case LLVMAlloca:
                    executor->locals->array[executor->locals->size].node = instruction;
                    executor->locals->array[executor->locals->size].index = executor->locals->size;
                    executor->locals->size += 1;
                    break;

                case LLVMLoad:
                    CompileLoad(out_file, instruction, executor, self);
                    break;

                case LLVMBr:
                    CompileBr(out_file, instruction, predicate, basic_block, executor, self);
                    break;

                case LLVMStore:
                    CompileStore(out_file, instruction, executor, self);
                    break;

                case LLVMICmp:
                    CompileCmp(out_file, instruction, &predicate, executor, self);
                    break;

                case LLVMCall:
                {
                    FixReg(executor->fixup, instruction);
                    Compile_func_args(out_file, instruction, calling_reg, executor, self);

                    LLVMValueRef call_function = LLVMGetCalledValue(instruction);

                    DUMP("    add r8, %ld; Emmited\n", executor->locals->size * VAR_SIZE);
                    Emit_add(self, r8, executor->locals->size * VAR_SIZE, true);
                    DUMP("    call %s(%lu); Emmited\n", LLVMGetValueName(call_function), GetFunctionLabel(executor->table, call_function));
                    Emit_call(self, GetFunctionLabel(executor->table, call_function));
                    DUMP("    sub r8, %ld; Emmited\n", executor->locals->size * VAR_SIZE);
                    Emit_sub(self, r8, (uint32_t)executor->locals->size * VAR_SIZE, true);

                    for (int index = MAX_REG_IN_USE - 1; index != -1; --index)
                    {
                        if (executor->fixup->array[index].node != nullptr)
                        {
                            DUMP("    pop %%%d\n", executor->fixup->array[index].reg);
                            Emit_pop(self, executor->fixup->array[index].reg, false);
                        }
                    }
                    if (LLVMGetCalledFunctionType(instruction) != LLVMVoidType())
                    {
                        DUMP("    mov %%%d, rax; Emmited\n\n", FindReg(executor->fixup, instruction));
                        if (FindReg(executor->fixup, instruction) != rax)
                            Emit_mov(self, FindReg(executor->fixup, instruction), rax, false);
                    }
                }
                    break;
                case LLVMRet:
                    CompileRet(out_file, instruction, start_label, func, executor, self);
                    break;

                default:
                    break;
                }
            }
            FreeFixup(executor->fixup);
        }
        executor->locals->size = 0;
        executor->callee_regs->size = 0;
    }
    puts("closing...");

    puts("succes emit");
    return self;
}
