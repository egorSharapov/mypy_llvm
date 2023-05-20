#pragma once
#include "../../JIT/JIT.hpp"



typedef struct 
{
    uint32_t   label_num;
    uint32_t   bin_size;

} Label;


typedef struct
{
    Label *labels;
    size_t labels_size;
    
} Labels_t;


typedef struct 
{
    Instruction_t **buffer;
    uint32_t rip;
    size_t size;
    size_t capacity;
    Label *labels;
    size_t labels_size;
    
} Execute_module;



Execute_module*   Execute_module_ctor (size_t capacity);
void Emit_push    (Execute_module *self, Regs base_reg, uint32_t index_i32);
void Emit_push    (Execute_module *self, Regs reg, bool uses_extend_reg = false);
void Emit_push    (Execute_module *self, uint32_t number);
void Emit_pop     (Execute_module *self, Regs reg, bool need_extend_reg = false);
void Emit_pop     (Execute_module *self, Regs base_reg, uint32_t index_i32);
void Emit_mov     (Execute_module *self, Regs reg, uint32_t i32, bool uses_extend_regs = 0);
void Emit_mov     (Execute_module *self, Regs reg, uint64_t i64, bool uses_extend_regs = 0);
void Emit_xor     (Execute_module *self, Regs first, Regs second);
void Emit_syscall (Execute_module *self);
void Emit_call    (Execute_module *self, uint32_t label);
void Emit_ret     (Execute_module *self);
void Emit_add     (Execute_module *self, Regs memory_reg, Regs second_reg);
void Emit_add     (Execute_module *self, Regs reg, uint32_t value, bool uses_extend_regs);
void Emit_jumps   (Execute_module *self, Instructions jump_opcode, uint8_t n, bool need_prefix = true);
void Emit_cmp     (Execute_module *self, Regs first, Regs second);
void Emit_sub     (Execute_module *self, Regs reg, uint32_t value, bool uses_extend_regs);
void Emit_sub     (Execute_module *self, Regs first, Regs second, bool uses_extend_regs);
void Emit_mov     (Execute_module *self, Regs m_reg, Regs reg);
void Emit_imul    (Execute_module *self, Regs reg, Regs m_reg);
void Emit_idiv    (Execute_module *self, Regs reg);
void Emit_mov     (Execute_module *self, Regs first, Regs second, bool uses_extend_reg);
void Emit_mov     (Execute_module *self, uint32_t index, Regs first);
void Emit_mov_m_i (Execute_module *self, uint32_t index, uint32_t i32);
//void Emit_mov_r_m (Execute_module *self, uint32_t index, uint32_t i32);
void Emit_mov_r_m (Execute_module *self, Regs reg, uint32_t index);
void Emit_add_r_r (Execute_module *self, Regs first, Regs second);
void Emit_cmp     (Execute_module *self, Regs first, uint32_t second);
void Emit_divss   (Execute_module *self, XRegs first, XRegs second);
void Emit_mulss   (Execute_module *self, XRegs first, XRegs second);
void Emit_cvtss2si (Execute_module *self, Regs first, XRegs second);
void Emit_cvtsi2ss (Execute_module *self, XRegs first, Regs second);
void Emit_neg     (Execute_module *self, Regs first);

void Make_elf (const char *elf_file_name, byte *exec_array);
byte *Write_module (Execute_module *self, size_t *binary_size);
void Sync_labels (Execute_module *self);
void Create_label (Execute_module *self, uint32_t label_num, uint32_t bin_size);
