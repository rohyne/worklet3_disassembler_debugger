
#include "watchpoint.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

/* Stores all watchpoints. */
static Watchpoint watchpoints[MAX_WATCHPOINTS];
static WatchReadMemory read_memory;

/* Convert watched bytes into a value. */
static int read_value(uint64_t address, size_t size,
                      uint64_t *value)
{
    unsigned char bytes[8] = {0};
    size_t i;

    if (read_memory == NULL || value == NULL ||
        size == 0 || size > sizeof(bytes)) {
        return -1;
    }

    if (read_memory(address, bytes, size) != 0) {
        return -1;
    }

    *value = 0;

    for (i = 0; i < size; i++) {
        *value |= (uint64_t)bytes[i] << (i * 8);
    }

    return 0;
}

/* Initialize all watchpoint slots. */
void watchpoint_init(WatchReadMemory reader)
{
    read_memory = reader;
    memset(watchpoints, 0, sizeof(watchpoints));
}

/* Add a watchpoint and save its initial value. */
int watchpoint_add(uint64_t address, size_t size)
{
    uint64_t value;
    int i;

    if (read_memory == NULL || size == 0 || size > 8 ||
        address > UINT64_MAX - (size - 1)) {
        return -1;
    }

    for (i = 0; i < MAX_WATCHPOINTS; i++) {
        if (watchpoints[i].enabled &&
            watchpoints[i].address == address) {
            printf("Watchpoint already exists.\n");
            return -1;
        }
    }

    for (i = 0; i < MAX_WATCHPOINTS; i++) {
        if (!watchpoints[i].enabled) {
            if (read_value(address, size, &value) != 0) {
                printf("Could not read watched memory.\n");
                return -1;
            }

            watchpoints[i].address = address;
            watchpoints[i].size = size;
            watchpoints[i].value = value;
            watchpoints[i].enabled = 1;

            printf("Watchpoint %d set at 0x%" PRIx64
                   " (%zu bytes), value = 0x%" PRIx64 "\n",
                   i + 1, address, size, value);

            return i + 1;
        }
    }

    printf("Maximum watchpoints reached.\n");
    return -1;
}

/* Remove a watchpoint by address. */
int watchpoint_remove(uint64_t address)
{
    int i;

    for (i = 0; i < MAX_WATCHPOINTS; i++) {
        if (watchpoints[i].enabled &&
            watchpoints[i].address == address) {
            watchpoints[i].enabled = 0;
            printf("Watchpoint %d removed.\n", i + 1);
            return 0;
        }
    }

    printf("Watchpoint not found.\n");
    return -1;
}

/* Remove a watchpoint by ID. */
int watchpoint_remove_id(int id)
{
    if (id < 1 || id > MAX_WATCHPOINTS ||
        !watchpoints[id - 1].enabled) {
        printf("Watchpoint ID not found.\n");
        return -1;
    }

    watchpoints[id - 1].enabled = 0;
    printf("Watchpoint %d removed.\n", id);
    return 0;
}

/* Check watched memory after an instruction. */
int watchpoint_check(uint64_t writer_pc)
{
    uint64_t current_value;
    int i;
    int hits = 0;

    if (read_memory == NULL) {
        return -1;
    }

    for (i = 0; i < MAX_WATCHPOINTS; i++) {
        if (!watchpoints[i].enabled) {
            continue;
        }

        if (read_value(watchpoints[i].address,
                       watchpoints[i].size,
                       &current_value) != 0) {
            printf("Could not read watchpoint %d.\n", i + 1);
            continue;
        }

        if (current_value != watchpoints[i].value) {
            printf("\n[Watchpoint %d] Memory changed\n", i + 1);
            printf("Address: 0x%08" PRIx64 "\n",
                   watchpoints[i].address);
            printf("Old value: 0x%08" PRIx64 "\n",
                   watchpoints[i].value);
            printf("New value: 0x%08" PRIx64 "\n",
                   current_value);
            printf("Written by: 0x%08" PRIx64 "\n",
                   writer_pc);

            watchpoints[i].value = current_value;
            hits++;
        }
    }

    return hits > 0 ? 1 : 0;
}

/* Display active watchpoints. */
void watchpoint_list(void)
{
    int i;
    int found = 0;

    for (i = 0; i < MAX_WATCHPOINTS; i++) {
        if (!watchpoints[i].enabled) {
            continue;
        }

        printf("ID: %d, Address: 0x%08" PRIx64
               ", Size: %zu, Value: 0x%08" PRIx64 "\n",
               i + 1,
               watchpoints[i].address,
               watchpoints[i].size,
               watchpoints[i].value);

        found = 1;
    }

    if (!found) {
        printf("No active watchpoints.\n");
    }
}
