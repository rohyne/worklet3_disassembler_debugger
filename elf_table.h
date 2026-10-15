#ifndef ELF_TABLE_H
#define ELF_TABLE_H

#include <stdio.h>
#include "disasm.h"

typedef enum SymType { SYM_FUNC, SYM_LABEL, SYM_OBJECT } SymType;

typedef struct SymEntry {
    unsigned int addr;
    char         name[32];
    SymType      type;
    unsigned int size;
    char         section[8];     /* ".text" or ".data" */
} SymEntry;

typedef struct SymTable {
    SymEntry    *entries;        /* sorted by address */
    unsigned int count;
} SymTable;

/* Derive a symbol table from decoded instructions (see rules in elf_table.c). */
SymTable    build_symbol_table(const DisasmResult *r);
void        free_symbol_table(SymTable *t);

/* addr -> name, or NULL if no symbol sits exactly at addr. */
const char *symtab_lookup(const SymTable *t, unsigned int addr);

/* Address | Bytes | Len | Instruction listing. `syms` may be NULL. */
void print_instruction_table(const DisasmResult *r, const SymTable *syms, FILE *out);
void print_symbol_table(const SymTable *t, FILE *out);

#endif