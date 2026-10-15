#include <stdio.h>
#include "disasm.h"

int main(void)
{
    DisasmResult r = disassemble_range(0x08048080, 100);
    char text[64];
    for (unsigned i = 0; i < r.count; i++) {
        const Instruction *in = &r.instructions[i];
        format_instruction(in, text, sizeof text);
        printf("0x%08X  ", in->addr);
        for (int b = 0; b < in->len; b++) printf("%02X ", in->raw_bytes[b]);
        printf("%*s %s\n", (5 - in->len) * 3, "", text);
    }
    free_disasm_result(&r);

    DisasmResult bad = disassemble_range(0x08048081, 5);   /* mid-instruction */
    printf("bad address -> count = %u\n", bad.count);
    return 0;
}