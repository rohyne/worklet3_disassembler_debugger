#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stdint.h>

// Functions supplied by the debugger
typedef struct {
    int (*set_breakpoint)(uint64_t address);
    int (*set_watchpoint)(uint64_t address);
    int (*run)(void);
    int (*step)(void);
    int (*continue_execution)(void);
    void (*show_registers)(void);
    void (*show_help)(void);
    void (*shutdown)(void);
} CliOps;

// Initializes the CLI
void cli_init(const CliOps *ops);

// Starts the command-line interface
void cli_run(void);

// Processes a single user command
bool cli_execute(const char *input);

#endif