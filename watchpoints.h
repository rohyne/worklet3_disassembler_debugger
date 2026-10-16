
#ifndef WATCHPOINT_H
#define WATCHPOINT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Maximum number of watchpoints supported.
#define MAX_WATCHPOINTS 16

// Stores the address and original value being monitored.
typedef struct {
    uint64_t address;
    uint64_t value;
    size_t size;
    bool enabled;
} Watchpoint;

// Reads memory at an address supplied by the debugger core.
typedef int (*WatchReadMemory)(
    uint64_t address,
    void *buffer,
    size_t size
);

// Initializes the watchpoint system.
void watchpoint_init(WatchReadMemory read_memory);

// Adds a watchpoint for a memory address.
int watchpoint_add(uint64_t address, size_t size);

// Removes a watchpoint at an address.
int watchpoint_remove(uint64_t address);

// Checks whether any monitored memory has changed.
int watchpoint_check(void);

#endif
