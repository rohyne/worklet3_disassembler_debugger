#include "cli.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLI_INPUT_SIZE 256

// Stores the debugger functions provided by the main progam
static CliOps cli_ops;
static bool cli_initialized = false;
static bool cli_running = true;

// Removes whitespace
static void trim(char *str)
{
    char *start = str;
    char *end;

    while (isspace((unsigned char)*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);

    end = str + strlen(str);

    while (end > str && isspace((unsigned char)*(end - 1)))
        end--;

    *end = '\0';
}

// Converts a hexadecimal or decimal address into an integer
static int parse_address(const char *str, uint64_t *address)
{
    char *end;
    unsigned long long value;

    if (str == NULL || *str == '\0' || *str == '-')
        return 0;

    value = strtoull(str, &end, 0);

    if (end == str || *end != '\0')
        return 0;

    *address = (uint64_t)value;
    return 1;
}

// Validates an address and forwards it to the debugger core
static bool dispatch_address_command(
    const char *argument,
    int (*callback)(uint64_t),
    const char *name)
{
    uint64_t address;

    if (argument == NULL || !parse_address(argument, &address)) {
        printf("Usage: %s <address>\n", name);
        return false;
    }

    if (callback == NULL || callback(address) != 0) {
        printf("Failed to execute %s.\n", name);
        return false;
    }

    printf("%s set at 0x%llx\n",
           name, (unsigned long long)address);

    return true;
}

// Connects the CLI to the debugger core
void cli_init(const CliOps *ops)
{
    if (ops == NULL) {
        cli_initialized = false;
        return;
    }

    cli_ops = *ops;
    cli_initialized = true;
    cli_running = true;
}

// Interprets and executes a user-entered command
bool cli_execute(const char *input)
{
    char buffer[CLI_INPUT_SIZE];
    char *command;
    char *argument;

    if (input == NULL)
        return false;

    if (strlen(input) >= sizeof(buffer)) {
        printf("Command is too long.\n");
        return true;
    }

    strcpy(buffer, input);
    trim(buffer);

    if (buffer[0] == '\0')
        return true;

    command = strtok(buffer, " \t");
    argument = strtok(NULL, " \t");

    // Displays available commands.
    if (strcmp(command, "help") == 0) {
        if (argument != NULL)
            printf("Usage: help\n");
        else if (cli_ops.show_help != NULL)
            cli_ops.show_help();
        else
            printf("Commands: help, break, watch, run, step, "
                   "continue, registers, quit\n");
    }
    // Sets a breakpoint at the specified address.
    else if (strcmp(command, "break") == 0) {
        dispatch_address_command(
            argument, cli_ops.set_breakpoint, "break");
    }
    // Requests a watchpoint at the specified address.
    else if (strcmp(command, "watch") == 0) {
        dispatch_address_command(
            argument, cli_ops.set_watchpoint, "watch");
    }
    // Starts program execution.
    else if (strcmp(command, "run") == 0) {
        if (argument != NULL)
            printf("Usage: run\n");
        else if (cli_ops.run != NULL)
            cli_ops.run();
        else
            printf("Run command is not connected.\n");
    }
    // Executes one instruction.
    else if (strcmp(command, "step") == 0) {
        if (argument != NULL)
            printf("Usage: step\n");
        else if (cli_ops.step != NULL)
            cli_ops.step();
        else
            printf("Step command is not connected.\n");
    }
    // Resumes execution after a pause.
    else if (strcmp(command, "continue") == 0) {
        if (argument != NULL)
            printf("Usage: continue\n");
        else if (cli_ops.continue_execution != NULL)
            cli_ops.continue_execution();
        else
            printf("Continue command is not connected.\n");
    }
    // Displays the current register values.
    else if (strcmp(command, "registers") == 0) {
        if (argument != NULL)
            printf("Usage: registers\n");
        else if (cli_ops.show_registers != NULL)
            cli_ops.show_registers();
        else
            printf("Register display is not connected.\n");
    }
    // Exits the debugger.
    else if (strcmp(command, "quit") == 0) {
        if (argument != NULL) {
            printf("Usage: quit\n");
        } else {
            cli_running = false;

            if (cli_ops.shutdown != NULL)
                cli_ops.shutdown();
        }
    }
    // Handles unrecognized commands.
    else {
        printf("Unknown command: %s\n", command);
        printf("Type 'help' for available commands.\n");
    }

    return cli_running;
}

// Repeatedly reads and executes commands.
void cli_run(void)
{
    char input[CLI_INPUT_SIZE];

    if (!cli_initialized) {
        printf("CLI is not initialized.\n");
        return;
    }

    cli_running = true;

    while (cli_running) {
        printf("debugger> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        // Rejects input that exceeds the buffer.
        if (strchr(input, '\n') == NULL && !feof(stdin)) {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            printf("Command is too long.\n");
            continue;
        }

        cli_execute(input);
    }
}