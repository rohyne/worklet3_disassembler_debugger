
#include "watchpoint.h"

#include <stdio.h>
#include <string.h>

// Stores all active watchpoints.
static Watchpoint watchpoints[MAX_WATCHPOINTS];

// Callback used to read memory through the debugger.
static WatchReadMemory memory_reader = NULL;

// Initializes the watchpoint table.
void watchpoint_init(WatchReadMemory read_memory)
{
    memset(watchpoints, 0, sizeof(watchpoints));
    memory_reader = read_memory;
}

// Adds a watchpoint and saves its initial memory value.
int watchpoint_add(uint64_t address, size_t size)
{
    size_t i;
    uint64_t value = 0;

    if (memory_reader == NULL || size == 0 ||
        size > sizeof(value)) {
        return -1;
    }

    // Read the value before monitoring begins.
    if (memory_reader(address, &value, size) != 0) {
        return -1;
    }

    // Find an unused slot.
    for (i = 0; i < MAX_WATCHPOINTS; i++) {
        if (!watchpoints[i].enabled) {
            watchpoints[i].address = address;
            watchpoints[i].value = value;
            watchpoints[i].size = size;
            watchpoints[i].enabled = true;

            return 0;
        }
    }

    return -1;
}

// Disables the watchpoint at the specified address.
int watchpoint_remove(uint64_t address)
{
    size_t i;

    for (i = 0; i < MAX_WATCHPOINTS; i++) {
        if (watchpoints[i].enabled &&
            watchpoints[i].address == address) {
            watchpoints[i].enabled = false;
            return 0;
        }
    }

    return -1;
}

// Compares current memory values with their saved values.
int watchpoint_check(void)
{
    size_t i;

    if (memory_reader == NULL)
        return -1;

    for (i = 0; i < MAX_WATCHPOINTS; i++) {
        uint64_t current = 0;

        if (!watchpoints[i].enabled)
            continue;

        // Read the latest value from the monitored address.
        if (memory_reader(
                watchpoints[i].address,
                &current,
                watchpoints[i].size) != 0) {
            return -1;
        }

        // Report a hit if the memory value has changed.
        if (current != watchpoints[i].value) {
            printf("Watchpoint hit at address 0x%llx\n",
                   (unsigned long long)watchpoints[i].address);

            printf("Old value: 0x%llx\n",
                   (unsigned long long)watchpoints[i].value);

            printf("New value: 0x%llx\n",
                   (unsigned long long)current);

            // Save the latest value for subsequent checks.
            watchpoints[i].value = current;

            return 1;
        }
    }

    return 0;
}
