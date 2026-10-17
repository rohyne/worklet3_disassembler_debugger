#include "display.h"
#include "loader.h"

#include <stdio.h>
#include <string.h>

const char *addr_str(uint32_t addr)
{
    //4 buffers so it can be used twice in one printf
    static char buf[4][96];
    static int slot = 0;
    char *out = buf[slot];
    slot = (slot + 1) % 4;

    uint32_t off;
    const char *name = addr_to_sym(addr, &off);

    if (!name)         snprintf(out, 96, "0x%08x", addr);
    else if (off == 0) snprintf(out, 96, "0x%08x <%s>", addr, name);
    else               snprintf(out, 96, "0x%08x <%s+%u>", addr, name, off);
    return out;
}

void show_regs(const DebuggerCore *dbg)
{
    const uint32_t *r = dbg->cpu.regs;

    //      <reg> = 0x<hex_value> (<decimal value>)
 
    printf("  EAX = 0x%08x (%d)\n", r[REG_EAX], (int)r[REG_EAX]);
    printf("  EBX = 0x%08x (%d)\n", r[REG_EBX], (int)r[REG_EBX]);
    printf("  ECX = 0x%08x (%d)\n", r[REG_ECX], (int)r[REG_ECX]);
    printf("  EDX = 0x%08x (%d)\n", r[REG_EDX], (int)r[REG_EDX]);
    printf("  ESI = 0x%08x   EDI = 0x%08x\n", r[REG_ESI], r[REG_EDI]);
    printf("  ESP = 0x%08x   EBP = 0x%08x\n", r[REG_ESP], r[REG_EBP]);
    // eip gets value from debugger engine
    printf("  EIP = %s\n", addr_str(dbg->cpu.eip));
    // flags
    printf("  flags: ZF=%d SF=%d CF=%d OF=%d\n",
           dbg->cpu.zero_flag, dbg->cpu.sign_flag,
           dbg->cpu.carry_flag, dbg->cpu.overflow_flag);
}

void show_mem(DebuggerCore *dbg, uint32_t addr, int len)
{
    //16 bytes per line
    for (int i = 0; i < len; i += 16) {
        printf("  %-28s", addr_str(addr + i));

        for (int j = i; j < len && j < i + 16; j++) {
            uint8_t b;
            if (read_memory(dbg, addr + j, &b, 1) == 0) printf(" %02x", b);
            else                                        printf(" ??");
        }
        printf("\n");
    }
}

//memory operand as text
static void mem_str(char *out, size_t cap, const MemOperand *m)
{
    if (m->base == REG_INVALID && m->index == REG_INVALID) {
        snprintf(out, cap, "[%s]", addr_str((uint32_t)m->disp));
        return;
    }

    int n = snprintf(out, cap, "[");
    if (m->base != REG_INVALID)
        n += snprintf(out + n, cap - n, "%s", reg_to_string(m->base));
    if (m->index != REG_INVALID)
        n += snprintf(out + n, cap - n, "+%s*%u", reg_to_string(m->index), m->scale);
    if (m->disp)
        n += snprintf(out + n, cap - n, "%+d", m->disp);
    snprintf(out + n, cap - n, "]");
}

void show_instr(const Instruction *in, bool current)
{
    char dst[112] = "", src[112] = "";

    //address and raw bytes
    printf("%s %-28s ", current ? "=>" : "  ", addr_str(in->start_address));
    for (size_t i = 0; i < 7; i++) {
        if (i < in->length) printf("%02x ", in->raw_decoded.raw_bytes[i]);
        else                printf("   ");
    }

    //first operand
    if (in->is_memory_dest)
        mem_str(dst, sizeof(dst), &in->mem_dest);
    else if (in->dest_reg != REG_INVALID)
        snprintf(dst, sizeof(dst), "%s", reg_to_string(in->dest_reg));

    //second operand
    if (in->is_memory_src)
        mem_str(src, sizeof(src), &in->mem_src);
    else if (in->has_immediate && strcmp(in->mnemonic, "int") == 0)
        snprintf(src, sizeof(src), "0x%x", (unsigned)in->immediate_val);
    else if (in->has_immediate)
        snprintf(src, sizeof(src), "%d", in->immediate_val);
    else if (in->src_reg != REG_INVALID)
        snprintf(src, sizeof(src), "%s", reg_to_string(in->src_reg));

    printf(" %-5s", in->mnemonic);

    if (in->type == JUMP || in->type == CONDITIONAL_BRANCH || in->type == CALL)
        printf(" %s", addr_str(in->target_address));
    else if (dst[0] && src[0]) printf(" %s, %s", dst, src);
    else if (dst[0])           printf(" %s", dst);
    else if (src[0])           printf(" %s", src);

    printf("\n");
}
