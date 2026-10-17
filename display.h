#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "debugger_core.h"

// address as symbol text
const char *addr_str(uint32_t addr);

void show_regs(const DebuggerCore *dbg);
void show_mem(DebuggerCore *dbg, uint32_t addr, int len);

// one disassembly line
// current line has a '=>' next to it
void show_instr(const Instruction *in, bool current);

#endif
