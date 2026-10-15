/* Builds the "ELF table" views (instruction listing + symbol table) from
 * Worklet 1's Instruction/DisasmResult structs.
 *
 * Symbols are DERIVED, because decoded instructions carry no names:
 *   - first instruction          -> _start       FUNC   size = code length
 *   - target of JMP/Jcc          -> loc_<addr>   LABEL  size 0
 *   - target of CALL             -> sub_<addr>   FUNC   size 0 (unknown)
 *   - absolute memory operand    -> data_<addr>  OBJECT size 4 (assumed dword)
 * Real names (loop_top, sum, ...) only exist in a binary's .symtab. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elf_table.h"

static int has_addr(const SymEntry *e, unsigned n, unsigned addr)
{
    for (unsigned i = 0; i < n; i++)
        if (e[i].addr == addr) return 1;
    return 0;
}

static void add_sym(SymEntry *e, unsigned *n, unsigned addr, const char *fmt,
                    SymType type, unsigned size, const char *section)
{
    if (has_addr(e, *n, addr)) return;              /* first writer wins (_start) */
    SymEntry *s = &e[(*n)++];
    s->addr = addr;
    snprintf(s->name, sizeof s->name, fmt, addr);
    s->type = type;
    s->size = size;
    snprintf(s->section, sizeof s->section, "%s", section);
}

static int cmp_addr(const void *a, const void *b)
{
    unsigned x = ((const SymEntry *)a)->addr, y = ((const SymEntry *)b)->addr;
    return (x > y) - (x < y);
}

SymTable build_symbol_table(const DisasmResult *r)
{
    SymTable t = { NULL, 0 };
    if (r->count == 0) return t;

    unsigned cap = r->count * 3 + 1;                /* each instr adds <= 3 symbols */
    SymEntry *e = calloc(cap, sizeof *e);
    unsigned n = 0;

    unsigned code_start = r->instructions[0].addr;
    const Instruction *last = &r->instructions[r->count - 1];
    unsigned code_end = last->addr + last->len;

    add_sym(e, &n, code_start, "_start", SYM_FUNC, code_end - code_start, ".text");

    for (unsigned i = 0; i < r->count; i++) {
        const Instruction *in = &r->instructions[i];

        int is_call = strcmp(in->cmd, "CALL") == 0;
        int is_jump = in->cmd[0] == 'J';
        if ((is_call || is_jump) && in->n_operands >= 1 && in->operands[0].type == OP_IMMEDIATE) {
            unsigned target = (unsigned)in->operands[0].immediate;
            if (target >= code_start && target < code_end)      /* skip targets outside the code */
                add_sym(e, &n, target, is_call ? "sub_%08X" : "loc_%08X",
                        is_call ? SYM_FUNC : SYM_LABEL, 0, ".text");
        }

        for (unsigned k = 0; k < in->n_operands; k++) {
            const Operand *o = &in->operands[k];
            if (o->type == OP_MEMORY && o->mem.base == NO_REG)
                add_sym(e, &n, (unsigned)o->mem.offset, "data_%08X", SYM_OBJECT, 4, ".data");
        }
    }

    qsort(e, n, sizeof *e, cmp_addr);
    t.entries = e;
    t.count = n;
    return t;
}

void free_symbol_table(SymTable *t)
{
    free(t->entries);
    t->entries = NULL;
    t->count = 0;
}

const char *symtab_lookup(const SymTable *t, unsigned int addr)
{
    for (unsigned i = 0; i < t->count; i++)
        if (t->entries[i].addr == addr) return t->entries[i].name;
    return NULL;
}

static const char *type_name(SymType t)
{
    return t == SYM_FUNC ? "FUNC" : t == SYM_LABEL ? "LABEL" : "OBJECT";
}

void print_instruction_table(const DisasmResult *r, const SymTable *syms, FILE *out)
{
    fprintf(out, "%-12s %-16s %-4s %s\n", "Address", "Bytes", "Len", "Instruction");
    for (unsigned i = 0; i < r->count; i++) {
        const Instruction *in = &r->instructions[i];
        char bytes[MAX_INSTR_BYTES * 3 + 1] = "";
        for (int b = 0; b < in->len; b++)
            snprintf(bytes + b * 3, 4, "%02X ", in->raw_bytes[b]);

        char text[64];
        format_instruction(in, text, sizeof text);

        const char *label = syms ? symtab_lookup(syms, in->addr) : NULL;
        fprintf(out, "0x%08X   %-16s %-4d %s", in->addr, bytes, in->len, text);
        if (label) fprintf(out, "   <- %s", label);
        fputc('\n', out);
    }
}

void print_symbol_table(const SymTable *t, FILE *out)
{
    fprintf(out, "%-14s %-12s %-7s %-5s %s\n", "Symbol", "Address", "Type", "Size", "Section");
    for (unsigned i = 0; i < t->count; i++) {
        const SymEntry *s = &t->entries[i];
        fprintf(out, "%-14s 0x%08X   %-7s %-5u %s\n",
                s->name, s->addr, type_name(s->type), s->size, s->section);
    }
}