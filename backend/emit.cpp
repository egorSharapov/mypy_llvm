#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "../../JIT/JIT.hpp"
#include "../include/emit.hpp"


static const uint64_t LIBRARY_SIZE = 641;
static const uint64_t LOAD_ADDRESS  = 0x400000;
static const uint64_t TEXT_SEGMENT = 0x401000;

#ifndef NDEBUF
#define DUMP(...) printf (__VA_ARGS__)
#else
#define DUMP(...) ;
#endif


Execute_module* Execute_module_ctor (size_t capacity)
{
    Execute_module *self = (Execute_module *) calloc (1, sizeof (Execute_module));
    self->buffer = (Instruction_t **) calloc (capacity, sizeof (Instruction_t *));
    self->size = 0;
    self->capacity = capacity;

    self->labels = (Label  *) calloc (capacity, sizeof (Label));
    self->labels_size = 0;
    return self;
}


static inline Instruction_t * New_cmd ()
{
    return (Instruction_t *) calloc (1, sizeof (Instruction_t));
}


static inline byte Gen_ModRM (byte mod, byte reg, byte r_m)
{
    byte ModRM = 0;
    ModRM |= mod << args::mod;
    ModRM |= reg << args::reg;
    ModRM |= r_m << args::r_m;
    return ModRM;
}


void Emit_cvtsi2ss (Execute_module *self, XRegs first, Regs second)
{
    Instruction_t *cvtsi2ss = New_cmd ();
    
    cvtsi2ss->xmmprefix = xmm_prefix;
    cvtsi2ss->prefix = support_64_bit_prefix;
    cvtsi2ss->oldprefix = old_prefix;
    cvtsi2ss->opcode = CVTSI2SS;
    cvtsi2ss->ModRM = Gen_ModRM (reg_reg, first, second);


    cvtsi2ss->bin_size = self->rip;
    self->rip += 5;

    self->buffer[self->size++] = cvtsi2ss;

}


void Emit_cvtss2si (Execute_module *self, Regs first, XRegs second)
{
    Instruction_t *cvtss2si = New_cmd ();
    
    cvtss2si->xmmprefix = xmm_prefix;
    cvtss2si->prefix = support_64_bit_prefix;
    cvtss2si->oldprefix = old_prefix;
    cvtss2si->opcode = CVTSS2SI;
    cvtss2si->ModRM = Gen_ModRM (reg_reg, first, second);
    cvtss2si->bin_size = self->rip;
    self->rip += 5;

    self->buffer[self->size++] = cvtss2si;

}


void Emit_neg (Execute_module *self, Regs first)
{
    Instruction_t *neg = New_cmd ();
    
    neg->REX = support_64_bit_prefix;
    neg->opcode = NEG_r64;
    neg->ModRM = Gen_ModRM (reg_reg, 0b011, first);
    neg->bin_size = self->rip;
    self->rip += 3;

    self->buffer[self->size++] = neg;
}

void Emit_mulss (Execute_module *self, XRegs first, XRegs second)
{
    Instruction_t *mulss = New_cmd ();
    
    mulss->xmmprefix = xmm_prefix;
    mulss->oldprefix = old_prefix;
    mulss->opcode = MULSS;
    mulss->ModRM = Gen_ModRM (reg_reg, first, second);
    mulss->bin_size = self->rip;
    self->rip += 4;

    self->buffer[self->size++] = mulss;
}


void Emit_divss (Execute_module *self, XRegs first, XRegs second)
{
    Instruction_t *divss = New_cmd ();
    
    divss->xmmprefix = xmm_prefix;
    divss->oldprefix = old_prefix;
    divss->opcode = DIVSS;
    divss->ModRM = Gen_ModRM (reg_reg, first, second);
    divss->bin_size = self->rip;
    self->rip += 4;

    self->buffer[self->size++] = divss;
}


void Emit_cmp (Execute_module *self, Regs first, uint32_t second)
{
    Instruction_t *cmp = New_cmd ();

    cmp->REX = support_64_bit_prefix;
    cmp->bin_size = self->rip;

    switch (first)
    {
        case rax:
            cmp->opcode = 0x3D;
            self->rip += 6;
            break;
        default:
            cmp->opcode = CMP_r64_i32;
            cmp->ModRM = Gen_ModRM (0b11, 0b111, first);
            self->rip += 7;
            break;

    }
    cmp->i32 = second;
    cmp->need_i32 = true;

    self->buffer[self->size++] = cmp;
}


void Emit_cmp (Execute_module *self, Regs first, Regs second)
{
    Instruction_t *cmp = New_cmd ();

    cmp->REX = support_64_bit_prefix;
    cmp->opcode = CMP_r64_r64;

    cmp->ModRM = Gen_ModRM (0b11, first, second);

    cmp->bin_size = self->rip;
    self->rip += 3;

    self->buffer[self->size++] = cmp;
}


void Emit_idiv (Execute_module *self, Regs reg)
{
    Instruction_t *idiv = New_cmd ();

    idiv->REX = support_64_bit_prefix;
    idiv->opcode = IDIV_r64;
    idiv->ModRM = Gen_ModRM (0b11, 0b111, reg);


    idiv->bin_size = self->rip;
    self->rip += 3;

    self->buffer[self->size++] = idiv; 
}


void Emit_jumps (Execute_module *self, Instructions jump_opcode, uint8_t compare_label_number, bool need_prefix)
{
    Instruction_t *jump = New_cmd ();


    jump->prefix = need_prefix ? 0x0F : 0;
    jump->opcode = jump_opcode;

    jump->label_number = compare_label_number;
    jump->need_i32 = true;

    jump->bin_size = self->rip;
    self->rip += need_prefix ? 6 : 5;

    self->buffer[self->size++] = jump;
}


void Emit_call (Execute_module *self, uint32_t label)
{
    Instruction_t *call = New_cmd ();
    
    call->opcode = CALL_near;
    call->label_number = label;
    call->i32 = 0xBEDA;
    call->need_i32 = true;

    call->bin_size = self->rip;
    self->rip += 5;

    self->buffer[self->size++] = call;
}



void Emit_imul (Execute_module *self, Regs reg, Regs m_reg)
{
    Instruction_t *imul = New_cmd ();

    imul->REX = support_64_bit_prefix;
    imul->prefix = 0x0F;
    imul->opcode = IMUL_r64_m64;
    imul->ModRM = Gen_ModRM (0b00, reg, m_reg);
    imul->SIB = 0x24;

    imul->bin_size = self->rip;
    self->rip += 5;

    self->buffer[self->size++] = imul;
}


void Emit_sub (Execute_module *self, Regs first, Regs second, bool uses_extend)
{
    Instruction_t *sub = New_cmd ();

    sub->REX = support_64_bit_prefix;
    sub->opcode = SUB_r64_r64;
    sub->ModRM  = Gen_ModRM (0b11, second, first);

    sub->bin_size = self->rip;
    self->rip += 3;

    self->buffer[self->size++] = sub;
}


void Emit_sub (Execute_module *self, Regs reg, uint32_t value, bool uses_extend_regs)
{
    Instruction_t *sub = New_cmd ();

    sub->REX = uses_extend_regs ? double_extend_regs_prefix : support_64_bit_prefix;
    sub->opcode = ADD_r64_i32;
    sub->ModRM  = Gen_ModRM (0b11, 0b101, reg);

    sub->i32 = value;
    sub->need_i32 = true;
    sub->bin_size = self->rip;
    self->rip += 7;

    self->buffer[self->size++] = sub;
}


void Emit_sub (Execute_module *self, Regs reg, Regs m_reg)
{
    Instruction_t *sub = New_cmd ();

    sub->REX = support_64_bit_prefix;
    sub->opcode = SUB_r64_m64;
    sub->ModRM  = Gen_ModRM (0b00, reg, m_reg);
    sub->SIB    = 0x24;

    sub->bin_size = self->rip;
    self->rip += 4;

    self->buffer[self->size++] = sub;
}

void Emit_add (Execute_module *self, Regs reg, uint32_t value, bool uses_extend_regs)
{
    Instruction_t *add = New_cmd ();
    
    add->REX = uses_extend_regs ? double_extend_regs_prefix : support_64_bit_prefix;
    add->opcode = ADD_r64_i32;
    add->ModRM = Gen_ModRM (0b11, 0, reg);

    add->i32 = value;
    add->need_i32 = true;

    add->bin_size = self->rip;
    self->rip += 7;

    self->buffer[self->size++] = add;

}

void Emit_add_r_r (Execute_module *self, Regs first, Regs second)
{
    Instruction_t *add = New_cmd ();

    add->REX = support_64_bit_prefix;
    add->opcode = ADD_r64_r64;
    add->ModRM = Gen_ModRM (0b11, second, first);

    add->bin_size = self->rip;
    self->rip += 3;

    self->buffer[self->size++] = add;
}

void Emit_add (Execute_module *self, Regs memory_reg, Regs second_reg)
{
    Instruction_t *add = New_cmd ();
    
    add->REX = support_64_bit_prefix;
    add->opcode = ADD_m64_r64;
    add->ModRM = Gen_ModRM (0b00, second_reg, memory_reg);

    add->SIB = 0x24;

    add->bin_size = self->rip;
    self->rip += 4;

    self->buffer[self->size++] = add;

}


void Emit_ret (Execute_module *self)
{
    Instruction_t *ret = New_cmd ();
    ret->opcode = RET;

    ret->bin_size = self->rip;
    self->rip += 1;
    self->buffer[self->size++] = ret;
}


void Emit_xor (Execute_module *self, Regs first, Regs second)
{
    Instruction_t *xor_x86 = New_cmd ();

    xor_x86->prefix = support_64_bit_prefix;
    xor_x86->opcode = XOR_r64_r64;

    xor_x86->ModRM = Gen_ModRM (0b11, first, second);
    xor_x86->bin_size = self->rip;
    
    self->rip += 3;
    self->buffer[self->size++] = xor_x86;
}


void Emit_syscall (Execute_module *self)
{
    Instruction_t *syscall = New_cmd ();
    syscall->prefix = 0x0F;
    syscall->opcode = SYSCALL;

    syscall->bin_size = self->rip;
    self->rip += 2;

    self->buffer[self->size++] = syscall;
}

void Emit_mov_r_m (Execute_module *self, Regs reg, uint32_t index)
{
    Instruction_t *mov = New_cmd ();

    mov->REX = double_extend_regs_prefix;
    mov->opcode = MOV_r64_m64;

    mov->ModRM = Gen_ModRM (0b10, reg, r8);
    mov->i32 = index;
    mov->need_i32 = true;
    mov->bin_size = self->rip;
    self->rip += 7;
    self->buffer[self->size++] = mov;
}


void Emit_mov_m_i (Execute_module *self, uint32_t index, uint32_t i32)
{
    Instruction_t *mov = New_cmd ();

    mov->REX = double_extend_regs_prefix;
    mov->opcode = MOV_m64_i64;

    mov->ModRM = Gen_ModRM (0b10, r8, r8);
    mov->i64 = i32;
    mov->i64 = (mov->i64 << 32) + index;
    mov->need_i64 = true;
    mov->bin_size = self->rip;
    self->rip += 11;
    self->buffer[self->size++] = mov;
}


void Emit_mov (Execute_module *self, uint32_t index, Regs first)
{
    Instruction_t *mov = New_cmd ();

    mov->REX = double_extend_regs_prefix;
    mov->opcode = MOV_r64_r64;

    mov->ModRM = Gen_ModRM (0b10, first, r8);
    mov->i32 = index;
    mov->need_i32 = true;

    mov->bin_size = self->rip;
    self->rip += 7;
    self->buffer[self->size++] = mov;
}


void Emit_mov (Execute_module *self, Regs first, Regs second, bool uses_extend_reg)
{
    Instruction_t *mov = New_cmd ();

    mov->REX = support_64_bit_prefix;
    mov->opcode = MOV_r64_r64;

    mov->ModRM = Gen_ModRM (0b11, second, first);

    mov->bin_size = self->rip;
    self->rip += 3;
    self->buffer[self->size++] = mov;
}


void Emit_mov (Execute_module *self, Regs reg, uint32_t i32, bool uses_extend_regs)
{
    Instruction_t *mov = New_cmd ();

    mov->REX = uses_extend_regs ? double_extend_regs_prefix : 0;
    mov->opcode = MOV_r64_i64 | reg;
    mov->i32 = i32; 
    mov->need_i32 = true;

    mov->bin_size = self->rip;
    self->rip += uses_extend_regs ? 6 : 5;
    self->buffer[self->size++] = mov;
}


void Emit_mov (Execute_module *self, Regs m_reg, Regs reg)
{
    Instruction_t *mov = New_cmd ();

    mov->REX = support_64_bit_prefix;
    mov->opcode = MOV_m64_r64;
    mov->ModRM = Gen_ModRM (0b00, reg, m_reg);

    mov->SIB = 0x24;

    mov->bin_size = self->rip;
    self->rip += 4;

    self->buffer[self->size++] = mov;
}


void Emit_mov (Execute_module *self, Regs reg, uint64_t i64, bool uses_extend_regs)
{
    Instruction_t *mov = New_cmd ();

    mov->REX = uses_extend_regs ? double_extend_regs_prefix : 0;
    mov->opcode = MOV_r64_i64 | reg;
    mov->i64 = i64; 
    mov->need_i64 = true;

    mov->bin_size = self->rip;
    self->rip += 10;

    self->buffer[self->size++] = mov;
}


void Emit_pop (Execute_module *self, Regs base_reg, uint32_t index_i32)
{
    Instruction_t *pop = New_cmd ();
    
    pop->REX = single_extend_reg_prefix;
    pop->opcode = POP_m64;
    pop->opcode |= reg;

    pop->ModRM = Gen_ModRM (reg_disp32, rax, base_reg);
    pop->i32 = index_i32;
    pop->need_i32 = true;

    pop->bin_size = self->rip;
    self->rip += 7;

    self->buffer[self->size++] = pop;

}


void Emit_pop (Execute_module *self, Regs reg, bool need_extend_reg)
{
    Instruction_t *pop = New_cmd ();
    
    pop->REX = need_extend_reg ? single_extend_reg_prefix : 0;
    pop->opcode = POP_r64;
    pop->opcode |= reg;

    pop->bin_size = self->rip;
    self->rip += need_extend_reg ? 2 : 1;

    self->buffer[self->size++] = pop;

}


void Emit_push (Execute_module *self, Regs base_reg, uint32_t index_i32)
{
    Instruction_t *push = New_cmd ();
    
    push->prefix = single_extend_reg_prefix;
    push->opcode = PUSH_m64;
    push->ModRM = Gen_ModRM (reg_disp32, rsi, base_reg);
    
    push->i32   = index_i32;
    push->need_i32 = true;

    push->bin_size = self->rip;
    self->rip += 7;

    self->buffer[self->size++] = push;
}


void Emit_push (Execute_module *self, Regs reg, bool uses_extend_reg)
{
    Instruction_t *push = New_cmd ();
    
    push->REX = uses_extend_reg ? single_extend_reg_prefix : 0; 
    push->opcode = PUSH_r64;
    push->opcode |= reg;

    push->bin_size = self->rip;
    self->rip += uses_extend_reg ? 2 : 1;
    self->buffer[self->size++] = push;
}


void Emit_push (Execute_module *self, uint32_t number)
{
    Instruction_t *push = New_cmd ();

    push->opcode = PUSH_i64;
    push->i32 = number;
    push->need_i32 = true;

    push->bin_size = self->rip;
    self->rip += 5;
    self->buffer[self->size++] = push;
}


//magic

#include <elf.h>

static void Add_lib  (const char *file_path, FILE *exec)
{
    assert (file_path);
    assert (exec);
    puts ("lib adding...");

    FILE *lib_file = fopen (file_path, "rb");

    byte lib[LIBRARY_SIZE] = {}; //init
    if (!lib_file)
    {
        printf ("lib open error\n");
    }
    size_t check_size = fread (lib, sizeof(byte), LIBRARY_SIZE, lib_file);
    fwrite (lib, 1, LIBRARY_SIZE, exec);
    if (check_size != LIBRARY_SIZE)
    {
        DUMP ("invalid fread size in function %s\n", __FUNCTION__);
        printf ("check size: %ld but read size: %ld\n", check_size, LIBRARY_SIZE);
    }
    fclose (lib_file);
}


static Elf64_Phdr Basic_init (Elf64_Word p_flags, Elf64_Off offset, Elf64_Addr addr)
{
    Elf64_Phdr self = 
    {
        .p_type   = 1          , /* [PT_LOAD] */
        .p_flags  = p_flags    , /* PF_R */
        .p_offset = offset     , /* (bytes into file) */
        .p_vaddr  = addr       , /* (virtual addr at runtime) */
        .p_paddr  = addr       , /* (physical addr at runtime) */
        .p_filesz = MAX_SIZE   , /* (bytes in file) */
        .p_memsz  = MAX_SIZE   , /* (bytes in mem at runtime) */
        .p_align  = 4096       , /* (min mem alignment in bytes) */
    };
    return self;
}



void Make_elf (const char *elf_file_name, byte *exec_array)
{
    byte zero_fill[0x4001] = {0};

    FILE *exec = fopen (elf_file_name, "w");
    if (!exec)
        puts ("execute file open error");

    fwrite (zero_fill, 0x4001, 1, exec);
    fseek (exec, 0x0, SEEK_SET);
    
    Elf64_Ehdr elf_header = 
    {
        .e_ident = { 
                ELFMAG0,
                ELFMAG1,
                ELFMAG2,
                ELFMAG3,
                ELFCLASS64,
                ELFDATA2LSB,
                EV_CURRENT,
                ELFOSABI_NONE,
                0x0,
        },
        .e_type      = ET_EXEC    , /* (ET_EXEC) */
        .e_machine   = EM_X86_64  , /* (EM_X86_64) */
        .e_version   = EV_CURRENT , /* (EV_CURRENT) */
        .e_entry     = TEXT_SEGMENT   , /* (start address at runtime) */
        .e_phoff     = 64         , /* (bytes into file) */
        .e_shoff     = 4328       , /* (bytes into file) */
        .e_flags     = 0x0        ,
        .e_ehsize    = 64         , /* (bytes) */
        .e_phentsize = 56         , /* (bytes) */
        .e_phnum     = 4          , /* (program headers) */
        .e_shentsize = 64         , /* (bytes) */
        .e_shnum     = 0          , /* (section headers) */
        .e_shstrndx  = 0  
    };

    Elf64_Phdr first_pg_header  = Basic_init (0x4, 0, LOAD_ADDRESS);
    Elf64_Phdr second_pg_header = Basic_init (0x5, 4096, TEXT_SEGMENT);
    Elf64_Phdr memory_header  = Basic_init (PF_R|PF_W, 4096*2, 0x402000);
    Elf64_Phdr library_header   = Basic_init (PF_R|PF_X, 4096*4, 0x404000);

    fwrite (&elf_header  , sizeof (elf_header)  , 1, exec);
    fwrite (&first_pg_header , sizeof (first_pg_header) , 1, exec);
    fwrite (&second_pg_header, sizeof (second_pg_header), 1, exec);
    fwrite (&memory_header , sizeof (memory_header) , 1, exec);
    fwrite (&library_header  , sizeof (library_header)  , 1, exec);
    
    fseek (exec, 0x1000, SEEK_SET);
    DUMP ("%ld\n", fwrite (exec_array, sizeof (byte), MAX_SIZE, exec));

    fseek (exec, 0x4000, SEEK_SET);
    Add_lib ("programs/lib.out", exec);

    fclose (exec);
}



static void Write_instruction (Instruction_t *cmd, byte *buffer, size_t *index)
{
    if (cmd->xmmprefix)
        buffer[(*index)++] = cmd->xmmprefix;
    if (cmd->REX)
        buffer[(*index)++] = cmd->REX;
    if (cmd->prefix)
        buffer[(*index)++] = cmd->prefix;
    if (cmd->oldprefix)
        buffer[(*index)++] = cmd->oldprefix;

    buffer[(*index)++] = cmd->opcode;

    if (cmd->ModRM)
        buffer[(*index)++] = cmd->ModRM;
    if (cmd->SIB)
        buffer[(*index)++] = cmd->SIB;
    if (cmd->i64 or cmd->need_i64)
    {
        puts ("  print i64");
        *((uint64_t *) (buffer + *index)) = cmd->i64;
        *index += 8;
    }
    if (cmd->need_i32)
    {
        puts ("  print i32");
        *((uint32_t *) (buffer + *index)) = cmd->i32;
        *index += 4;
    }
}


byte *Write_module (Execute_module *self, size_t *binary_size)
{
    size_t rip = 0;

    byte *exec_buffer = (byte *) calloc (1, self->size*sizeof(Instruction_t));

    for (size_t ind = 0; ind < self->size; ind++)
    {
        DUMP ("cmd[%2ld] size: %lx\n", ind + 1, self->buffer[ind]->bin_size);
        Write_instruction (self->buffer[ind], exec_buffer, &rip);
    }
    *binary_size = rip;
    return exec_buffer;
}


static uint32_t NEAR_CALL_i32_SIZE = 5;
static uint32_t NEAR_JCC_i32_SIZE = 6;


static uint32_t Search_label (Execute_module *self, uint32_t label_num)
{
    for (size_t index = 0; index < self->labels_size; index++)
    {
        if (self->labels[index].label_num == label_num)
            return self->labels[index].bin_size;
    }
    return -1; 
}

#ifndef NDEBUG
static void Dump_labels (Execute_module *self)
{
    puts ("\nDUMP LABELS");
    for (size_t index = 0; index < self->labels_size; index++)
        DUMP ("label: %d with size: %d\n", 
                        self->labels[index].label_num,
                                      self->labels[index].bin_size);
    puts ("\n");
}
#endif


void Sync_labels (Execute_module *self)
{
    uint32_t relative_addr = 0;
    uint32_t absolute_addr = 0;

    #ifndef NDEBUG
    Dump_labels (self);
    #endif

    for (size_t rip = 0; rip < self->size; rip++)
    {
        switch (self->buffer[rip]->opcode)
        {
        case CALL_near: case JMP:
        case JBE: case JAE:
        case JNE: case JE:
        case JLE: case JGE:
            absolute_addr = Search_label (self, self->buffer[rip]->label_number);
            relative_addr = absolute_addr - self->buffer[rip]->bin_size;
            
            switch (self->buffer[rip]->opcode)
            {
            case CALL_near: case JMP:
                relative_addr -= NEAR_CALL_i32_SIZE;
                break;
            case JBE: case JAE:
            case JNE: case JE:
            case JLE: case JGE:
                relative_addr -= NEAR_JCC_i32_SIZE;
                break;
            default:
                break;
            }
            self->buffer[rip]->i32 = relative_addr;
            self->buffer[rip]->need_i32 = true;
            DUMP ("label: %d ", self->buffer[rip]->label_number);
            DUMP (" absolute address: %d relative: %d\n", absolute_addr, relative_addr);
            break;
        default:
            break;
        }
    }
    puts ("");
}


void Create_label (Execute_module *self, uint32_t label_num, uint32_t bin_size)
{
    Label *compare_label = &self->labels[self->labels_size];
    compare_label->label_num = label_num;
    compare_label->bin_size = bin_size;
    self->labels_size+= 1;
}
