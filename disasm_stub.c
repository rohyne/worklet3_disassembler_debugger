/* DUMMY Worklet 1: returns a hardcoded disassembly of the sum-loop example.
 * No real decoding happens here. Replace with the real decoder later. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disasm.h"

static const char *REG_NAMES[] = {"EAX","EBX","ECX","EDX","ESI","EDI","EBP","ESP"};

static Operand reg_op(Register r)            { Operand o; memset(&o,0,sizeof o); o.type=OP_REGISTER;  o.reg=r; return o; }
static Operand imm_op(int v)                 { Operand o; memset(&o,0,sizeof o); o.type=OP_IMMEDIATE; o.immediate=v; return o; }
static Operand mem_op(Register b, int off)   { Operand o; memset(&o,0,sizeof o); o.type=OP_MEMORY; o.mem.base=b; o.mem.offset=off; return o; }

static Instruction mk(unsigned addr, const uint8_t *bytes, int len, const char *cmd,
                      unsigned n_ops, Operand a, Operand b)
{
    Instruction in;
    memset(&in, 0, sizeof in);
    in.addr = addr;
    memcpy(in.raw_bytes, bytes, len);
    in.len = len;
    strncpy(in.cmd, cmd, MAX_CMD_LEN - 1);
    in.n_operands = n_ops;
    in.operands[0] = a;
    in.operands[1] = b;
    return in;
}

#define PROGRAM_SIZE 9

static void build_program(Instruction p[PROGRAM_SIZE])
{
    static const uint8_t b0[] = {0xB8,0x00,0x00,0x00,0x00};
    static const uint8_t b1[] = {0xB9,0x00,0x00,0x00,0x00};
    static const uint8_t b2[] = {0x83,0xF9,0x05};
    static const uint8_t b3[] = {0x7D,0x05};
    static const uint8_t b4[] = {0x01,0xC8};
    static const uint8_t b5[] = {0x41};
    static const uint8_t b6[] = {0xEB,0xF6};
    static const uint8_t b7[] = {0xA3,0x20,0xA0,0x04,0x08};
    static const uint8_t b8[] = {0xF4};
    Operand none = imm_op(0);

    p[0] = mk(0x08048080, b0, 5, "MOV", 2, reg_op(EAX), imm_op(0));
    p[1] = mk(0x08048085, b1, 5, "MOV", 2, reg_op(ECX), imm_op(0));
    p[2] = mk(0x0804808A, b2, 3, "CMP", 2, reg_op(ECX), imm_op(5));
    p[3] = mk(0x0804808D, b3, 2, "JGE", 1, imm_op(0x08048094), none);
    p[4] = mk(0x0804808F, b4, 2, "ADD", 2, reg_op(EAX), reg_op(ECX));
    p[5] = mk(0x08048091, b5, 1, "INC", 1, reg_op(ECX), none);
    p[6] = mk(0x08048092, b6, 2, "JMP", 1, imm_op(0x0804808A), none);
    p[7] = mk(0x08048094, b7, 5, "MOV", 2, mem_op(NO_REG, 0x0804A020), reg_op(EAX));
    p[8] = mk(0x08048099, b8, 1, "HLT", 0, none, none);
}

DisasmResult disassemble_range(unsigned int start_addr, unsigned int count)
{
    Instruction prog[PROGRAM_SIZE];
    DisasmResult r = { NULL, 0 };
    unsigned int first = PROGRAM_SIZE;

    build_program(prog);
    for (unsigned i = 0; i < PROGRAM_SIZE; i++)
        if (prog[i].addr == start_addr) { first = i; break; }
    if (first == PROGRAM_SIZE) return r;            /* address not at an instruction boundary */

    r.count = PROGRAM_SIZE - first;
    if (count < r.count) r.count = count;
    r.instructions = malloc(r.count * sizeof(Instruction));
    memcpy(r.instructions, &prog[first], r.count * sizeof(Instruction));
    return r;
}

void free_disasm_result(DisasmResult *r)
{
    free(r->instructions);
    r->instructions = NULL;
    r->count = 0;
}

static void fmt_operand(const Operand *o, char *buf, unsigned size)
{
    switch (o->type) {
    case OP_REGISTER:  snprintf(buf, size, "%s", REG_NAMES[o->reg]); break;
    case OP_IMMEDIATE: snprintf(buf, size, "0x%X", (unsigned)o->immediate); break;
    case OP_MEMORY:
        if (o->mem.base == NO_REG) snprintf(buf, size, "[0x%X]", (unsigned)o->mem.offset);
        else                       snprintf(buf, size, "[%s%+d]", REG_NAMES[o->mem.base], o->mem.offset);
        break;
    }
}

void format_instruction(const Instruction *in, char *buf, unsigned int size)
{
    char a[32], b[32];
    int n = snprintf(buf, size, "%s", in->cmd);
    for (unsigned i = 0; i < in->n_operands && n < (int)size; i++) {
        fmt_operand(&in->operands[i], i == 0 ? a : b, 32);
        n += snprintf(buf + n, size - n, "%s%s", i == 0 ? " " : ", ", i == 0 ? a : b);
    }
}