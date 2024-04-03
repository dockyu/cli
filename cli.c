#include <stdio.h>

#include "console.h"
#include "report.h"

static bool do_jack(int argc, char *argv[])
{
    char *something = (argc >= 2) ? argv[1] : "";
    printf("%s jack\n", something);
    return true;
}



static void console_init()
{
    ADD_COMMAND(jack, "Say something to jack", "[something]");
}

int main(int argc, char *argv[])
{
    init_cmd();
    console_init();
    run_console();
}