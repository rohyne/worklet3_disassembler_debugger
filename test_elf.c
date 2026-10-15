#include <stdio.h>
#include "elf_table.h"

int main(void)
{
    DisasmResult r = disassemble_range(0x08048080, 100);
    SymTable syms = build_symbol_table(&r);

    printf("=== Instruction table ===\n");
    print_instruction_table(&r, &syms, stdout);
    printf("\n=== Symbol table ===\n");
    print_symbol_table(&syms, stdout);

    free_symbol_table(&syms);
    free_disasm_result(&r);
    return 0;
}