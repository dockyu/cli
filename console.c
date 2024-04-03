/* Implementation of simple command-line interface */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#include "linenoise.h"
#include "console.h"
#include "report.h"

char *line;
static char *prompt = "cmd> ";
int async = 0;

static cmd_element_t *cmd_list = NULL;

/* Add a new command */
void add_cmd(char *name, cmd_func_t operation, char *summary, char *parameter)
{
    cmd_element_t *next_cmd = cmd_list;
    cmd_element_t **last_loc = &cmd_list;
    while (next_cmd && strcmp(name, next_cmd->name) > 0) {
        last_loc = &next_cmd->next;
        next_cmd = next_cmd->next;
    }

    cmd_element_t *cmd = malloc_or_fail(sizeof(cmd_element_t), "add_cmd");
    cmd->name = name;
    cmd->operation = operation;
    cmd->summary = summary;
    cmd->param = parameter;
    cmd->next = next_cmd;
    *last_loc = cmd;
}

static bool do_help(int argc, char *argv[])
{
    printf("Commands:\n");
    cmd_element_t *clist = cmd_list;
    while (clist) {
        // printf("%s  %s| %s\n", clist->name, clist->param, clist->summary);
        printf("%-12s%-13s| %s\n", clist->name, clist->param, clist->summary);
        clist = clist->next;
    }

    return true;
}

void init_cmd()
{
    cmd_list = NULL;

    ADD_COMMAND(help, "Show summary", "");
}

void completion(const char *buf, linenoiseCompletions *lc) {
    if (buf[0] == 'h') {
        linenoiseAddCompletion(lc,"hello");
        linenoiseAddCompletion(lc,"hello there");
    }
}

char *hints(const char *buf, int *color, int *bold) {
    if (!strcasecmp(buf,"hello")) {
        *color = 35;
        *bold = 0;
        return " World";
    }
    return NULL;
}

bool run_console()
{
    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad(".cmd_history"); /* Load the history at startup */

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to linenoise() will block as long as the user types something
     * and presses enter.
     *
     * The typed string is returned as a malloc() allocated string by
     * linenoise, so the user needs to free() it. */

    while(1) {
        if (!async) {
            line = linenoise(prompt);
            if (line == NULL) break;
        } else {
            /* Asynchronous mode using the multiplexing API: wait for
             * data on stdin, and simulate async data coming from some source
             * using the select(2) timeout. */
            struct linenoiseState ls;
            char buf[1024];
            linenoiseEditStart(&ls,-1,-1,buf,sizeof(buf),prompt);
            while(1) {
		fd_set readfds;
		struct timeval tv;
		int retval;

		FD_ZERO(&readfds);
		FD_SET(ls.ifd, &readfds);
		tv.tv_sec = 1; // 1 sec timeout
		tv.tv_usec = 0;

		retval = select(ls.ifd+1, &readfds, NULL, NULL, &tv);
		if (retval == -1) {
		    perror("select()");
                    exit(1);
		} else if (retval) {
		    line = linenoiseEditFeed(&ls);
                    /* A NULL return means: line editing is continuing.
                     * Otherwise the user hit enter or stopped editing
                     * (CTRL+C/D). */
                    if (line != linenoiseEditMore) break;
		} else {
		    // Timeout occurred
                    static int counter = 0;
                    linenoiseHide(&ls);
		    printf("Async output %d.\n", counter++);
                    linenoiseShow(&ls);
		}
            }
            linenoiseEditStop(&ls);
            if (line == NULL) exit(0); /* Ctrl+D/C. */
        }

        /* Do something with the string. */
        if (line[0] != '\0' && line[0] != '/') {
            linenoiseHistoryAdd(line); /* Add to the history. */
            linenoiseHistorySave(".cmd_history"); /* Save the history on disk. */

            /* Translate line to argc, argv[] */
            char *token;
            char *argv[10];
            int argc = 0;
            token = strtok(line, " ");
            while (token != NULL && argc < 10) {
                argv[argc++] = token;
                token = strtok(NULL, " ");
            }

            bool find = false;
            cmd_element_t *now_cmd = cmd_list;
            while (now_cmd != NULL) {
                if (strcmp(now_cmd->name, argv[0]) == 0) {
                    find = true;
                    break;
                }
                now_cmd = now_cmd->next;
            }

            /* Call function if find */
            if (find) {
                now_cmd->operation(argc, argv);
            }
        } else if (!strncmp(line,"/historylen",11)) {
            /* The "/historylen" command will change the history len. */
            int len = atoi(line+11);
            linenoiseHistorySetMaxLen(len);
        } else if (!strncmp(line, "/mask", 5)) {
            linenoiseMaskModeEnable();
        } else if (!strncmp(line, "/unmask", 7)) {
            linenoiseMaskModeDisable();
        } else if (line[0] == '/') {
            printf("Unreconized command: %s\n", line);
        }
        free(line);
    }
    return 0;
}