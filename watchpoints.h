
#ifndef WATCHPOINT_H
#define WATCHPOINT_H

#include <stdint.h>
#include <stddef.h>

#define MAX_WATCHPOINTS 16

/* Reads bytes from debugger memory. */
typedef int (*WatchReadMemory)(uint64_t address,
                               void *buffer,
                               size_t size);

typedef struct {
    uint64_t address;
    uint64_t value;
    size_t size;
    int enabled;
} Watchpoint;

void watchpoint_init(WatchReadMemory reader);
int watchpoint_add(uint64_t address, size_t size);
int watchpoint_remove(uint64_t address);
int watchpoint_remove_id(int id);
int watchpoint_check(uint64_t writer_pc);
void watchpoint_list(void);

#endif
