
#ifndef CLI_H
#define CLI_H

#include <stdint.h>
#include <stddef.h>
#include "symtable.h"

typedef struct {
    int (*set_breakpoint)(uint64_t address);
    int (*set_watchpoint)(uint64_t address);
    int (*run)(void);
    int (*step)(void);
    int (*continue_execution)(void);
    void (*show_registers)(void);
    void (*show_help)(void);
    void (*shutdown)(void);

    /* Optional display callbacks. */
    void (*show_flags)(void);
    void (*show_memory)(uint64_t address, size_t length);
    void (*show_asm)(void);
    void (*show_register)(const char *name);
    void (*list_breakpoints)(void);
} CliOps;

void cli_init(const CliOps *ops, const SymTable *symtab);
void cli_run(void);
int cli_execute(const char *command);

#endif
