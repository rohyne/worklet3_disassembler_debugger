
#include "cli.h"
#include "watchpoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>

static CliOps cli_ops;
static const SymTable *cli_symtab;

/* Convert an address or symbol into an address. */
static int parse_address(const char *str, uint64_t *address)
{
    char *end;
    unsigned long long value;
    int base;

    if (str == NULL || address == NULL || *str == '\0' ||
        *str == '-') {
        return 0;
    }

    if (cli_symtab != NULL) {
        unsigned int sym_addr;

        if (symtab_lookup_name(cli_symtab, str, &sym_addr)) {
            *address = sym_addr;
            return 1;
        }
    }

    base = (str[0] == '0' &&
            (str[1] == 'x' || str[1] == 'X')) ? 16 : 10;

    errno = 0;
    value = strtoull(str, &end, base);

    if (errno == ERANGE || end == str || *end != '\0') {
        return 0;
    }

    *address = (uint64_t)value;
    return 1;
}

/* Split a command into arguments. */
static int tokenize(char *str, char *argv[], int max_args)
{
    int count = 0;
    char *p = str;

    while (*p) {
        while (isspace((unsigned char)*p)) {
            p++;
        }

        if (*p == '\0') {
            break;
        }

        if (count == max_args) {
            return -1;
        }

        argv[count++] = p;

        while (*p && !isspace((unsigned char)*p)) {
            p++;
        }

        if (*p) {
            *p++ = '\0';
        }
    }

    return count;
}

/* Print available commands. */
static void print_help(void)
{
    printf("Commands:\n");
    printf("  help\n");
    printf("  break <address|symbol>\n");
    printf("  watch <address|symbol>\n");
    printf("  delete <watchpoint_id>\n");
    printf("  info breakpoints\n");
    printf("  info watchpoints\n");
    printf("  run\n");
    printf("  step [count]\n");
    printf("  continue\n");
    printf("  print regs\n");
    printf("  print flags\n");
    printf("  print mem <address|symbol> [length]\n");
    printf("  print asm\n");
    printf("  print <register>\n");
    printf("  quit\n");
}

/* Initialize the CLI. */
void cli_init(const CliOps *ops, const SymTable *symtab)
{
    if (ops != NULL) {
        cli_ops = *ops;
    } else {
        memset(&cli_ops, 0, sizeof(cli_ops));
    }

    cli_symtab = symtab;
}

/* Execute one command. */
int cli_execute(const char *command)
{
    char buffer[256];
    char *argv[5];
    int argc;
    uint64_t address;

    if (command == NULL || strlen(command) >= sizeof(buffer)) {
        printf("Invalid command.\n");
        return -1;
    }

    strcpy(buffer, command);
    argc = tokenize(buffer, argv, 5);

    if (argc < 0) {
        printf("Too many arguments.\n");
        return -1;
    }

    if (argc == 0) {
        return 0;
    }

    if (strcmp(argv[0], "help") == 0) {
        if (argc != 1) {
            printf("Usage: help\n");
            return -1;
        }

        if (cli_ops.show_help) {
            cli_ops.show_help();
        } else {
            print_help();
        }
    }

    else if (strcmp(argv[0], "break") == 0 ||
             strcmp(argv[0], "watch") == 0) {
        int is_break = strcmp(argv[0], "break") == 0;

        if (argc != 2 || !parse_address(argv[1], &address)) {
            printf("Usage: %s <address|symbol>\n", argv[0]);
            return -1;
        }

        if (is_break) {
            if (cli_ops.set_breakpoint) {
                return cli_ops.set_breakpoint(address) < 0 ? -1 : 0;
            }
        } else {
            if (cli_ops.set_watchpoint) {
                return cli_ops.set_watchpoint(address) < 0 ? -1 : 0;
            }
        }

        printf("Command is not connected.\n");
        return -1;
    }

    else if (strcmp(argv[0], "delete") == 0) {
        char *end;
        long id;

        if (argc != 2 || *argv[1] == '-') {
            printf("Usage: delete <watchpoint_id>\n");
            return -1;
        }

        errno = 0;
        id = strtol(argv[1], &end, 10);

        if (errno == ERANGE || end == argv[1] ||
            *end != '\0' || id < 1 || id > MAX_WATCHPOINTS) {
            printf("Invalid watchpoint ID.\n");
            return -1;
        }

        return watchpoint_remove_id((int)id);
    }

    else if (strcmp(argv[0], "info") == 0) {
        if (argc != 2) {
            printf("Usage: info <breakpoints|watchpoints>\n");
            return -1;
        }

        if (strcmp(argv[1], "watchpoints") == 0) {
            watchpoint_list();
        } else if (strcmp(argv[1], "breakpoints") == 0) {
            if (cli_ops.list_breakpoints) {
                cli_ops.list_breakpoints();
            } else {
                printf("Breakpoint listing is not connected.\n");
                return -1;
            }
        } else {
            printf("Unknown info command.\n");
            return -1;
        }
    }

    else if (strcmp(argv[0], "run") == 0 ||
             strcmp(argv[0], "continue") == 0) {
        int result;

        if (argc != 1) {
            printf("Unexpected arguments.\n");
            return -1;
        }

        if (strcmp(argv[0], "run") == 0) {
            if (!cli_ops.run) {
                printf("Run is not connected.\n");
                return -1;
            }
            result = cli_ops.run();
        } else {
            if (!cli_ops.continue_execution) {
                printf("Continue is not connected.\n");
                return -1;
            }
            result = cli_ops.continue_execution();
        }

        return result;
    }

    else if (strcmp(argv[0], "step") == 0) {
        long count = 1;
        long i;
        char *end;

        if (argc > 2) {
            printf("Usage: step [count]\n");
            return -1;
        }

        if (argc == 2) {
            if (*argv[1] == '-') {
                printf("Invalid step count.\n");
                return -1;
            }

            errno = 0;
            count = strtol(argv[1], &end, 10);

            if (errno == ERANGE || end == argv[1] ||
                *end != '\0' || count < 1 || count > 100000) {
                printf("Invalid step count.\n");
                return -1;
            }
        }

        if (!cli_ops.step) {
            printf("Step is not connected.\n");
            return -1;
        }

        for (i = 0; i < count; i++) {
            int result = cli_ops.step();

            if (result != 0) {
                return result;
            }
        }
    }

    else if (strcmp(argv[0], "print") == 0) {
        if (argc == 2 && strcmp(argv[1], "regs") == 0) {
            if (cli_ops.show_registers) {
                cli_ops.show_registers();
            } else {
                printf("Register display is not connected.\n");
                return -1;
            }
        }

        else if (argc == 2 && strcmp(argv[1], "flags") == 0) {
            if (cli_ops.show_flags) {
                cli_ops.show_flags();
            } else {
                printf("Flag display is not connected.\n");
                return -1;
            }
        }

        else if (argc == 2 && strcmp(argv[1], "asm") == 0) {
            if (cli_ops.show_asm) {
                cli_ops.show_asm();
            } else {
                printf("Assembly display is not connected.\n");
                return -1;
            }
        }

        else if ((argc == 3 || argc == 4) &&
                 strcmp(argv[1], "mem") == 0) {
            char *end;
            unsigned long length = 4;

            if (!parse_address(argv[2], &address)) {
                printf("Invalid memory address.\n");
                return -1;
            }

            if (argc == 4) {
                if (*argv[3] == '-') {
                    printf("Invalid memory length.\n");
                    return -1;
                }

                errno = 0;
                length = strtoul(argv[3], &end, 10);

                if (errno == ERANGE || end == argv[3] ||
                    *end != '\0' || length < 1 || length > 256) {
                    printf("Invalid memory length.\n");
                    return -1;
                }
            }

            if (!cli_ops.show_memory) {
                printf("Memory display is not connected.\n");
                return -1;
            }

            cli_ops.show_memory(address, (size_t)length);
        }

        else if (argc == 2) {
            if (cli_ops.show_register) {
                cli_ops.show_register(argv[1]);
            } else {
                printf("Single-register display is not connected.\n");
                return -1;
            }
        }

        else {
            printf("Invalid print command.\n");
            return -1;
        }
    }

    else if (strcmp(argv[0], "quit") == 0) {
        if (argc != 1) {
            printf("Usage: quit\n");
            return -1;
        }

        if (cli_ops.shutdown) {
            cli_ops.shutdown();
        }

        return 1;
    }

    else {
        printf("Unknown command. Type 'help' for commands.\n");
        return -1;
    }

    return 0;
}

/* Read commands until the user quits. */
void cli_run(void)
{
    char command[256];

    while (1) {
        printf("(dbg) ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        if (strchr(command, '\n') == NULL && !feof(stdin)) {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF) {
                /* Discard the rest of the line. */
            }

            printf("Command is too long.\n");
            continue;
        }

        if (cli_execute(command) == 1) {
            break;
        }
    }
}
