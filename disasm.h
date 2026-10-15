#ifndef DISASM_H
#define DISASM_H

#include <stdint.h>

#define MAX_INSTR_BYTES 15
#define MAX_CMD_LEN     16
#define MAX_OPERANDS    3

typedef enum Register {
    EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP,
    NO_REG                      /* added: needed for [absolute address] operands */
} Register;

typedef enum OperandType {
    OP_REGISTER,
    OP_IMMEDIATE,
    OP_MEMORY
} OperandType;

typedef struct Operand {
    OperandType type;           /* addressing mode */
    union {
        Register reg;           /* if type == OP_REGISTER */
        int      immediate;     /* if type == OP_IMMEDIATE (also jump/call targets) */
        struct {                /* if type == OP_MEMORY: [base + offset] */
            Register base;      /* NO_REG means absolute address, offset is the address */
            int      offset;
        } mem;
    };
} Operand;

typedef struct Instruction {
    unsigned int addr;                      /* address in memory */
    uint8_t      raw_bytes[MAX_INSTR_BYTES];/* actual bytes, e.g. {0xB8, 0x00, ...} */
    int          len;                       /* length of instruction in bytes */
    char         cmd[MAX_CMD_LEN];          /* "MOV", "JMP", ... */
    Operand      operands[MAX_OPERANDS];
    unsigned int n_operands;
} Instruction;

typedef struct DisasmResult {
    Instruction *instructions;              /* heap array, free with free_disasm_result() */
    unsigned int count;                     /* no. of instructions */
} DisasmResult;

/* Disassemble up to `count` instructions starting at `start_addr`. */
DisasmResult disassemble_range(unsigned int start_addr, unsigned int count);
void         free_disasm_result(DisasmResult *r);

/* Formats one instruction as text, e.g. "MOV EAX, 0x0" */
void format_instruction(const Instruction *in, char *buf, unsigned int buf_size);

#endif