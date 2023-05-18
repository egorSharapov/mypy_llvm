#include <malloc.h>
#include <unistd.h>
#include "../include/ir2asm.hpp"
#include "../include/emit.hpp"

const char *ir_file_name  = "out/IR.bc";
const char *out_file_name = "out.asm";


int main ()
{
    size_t bin_size = 0;
    Execute_module *module = Create_asm (ir_file_name, out_file_name);

    Sync_labels (module);
    byte *exec_buffer = Write_module (module, &bin_size);
    Make_elf ("test.out", exec_buffer);
    return 0;
}