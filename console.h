#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>

#define HISTORY_FILE ".cmd_history"

/* Each command defined in terms of a function */
typedef bool (*cmd_func_t)(int argc, char *argv[]);

/* Organized as linked list in alphabetical order */
typedef struct __cmd_element {
    char *name;
    cmd_func_t operation;
    char *summary;
    char *param;
    struct __cmd_element *next;
} cmd_element_t;

/* Initialize interpreter */
void init_cmd();

bool run_console();

/* Add a new command */
void add_cmd(char *name, cmd_func_t operation, char *summary, char *parameter);
#define ADD_COMMAND(cmd, msg, param) add_cmd(#cmd, do_##cmd, msg, param)

#endif /* CONSOLE_H */