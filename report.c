#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "report.h"

/* Call malloc & exit if fails */
void *malloc_or_fail(size_t bytes, const char *fun_name)
{
    void *p = malloc(bytes);
    if (!p) {
        printf("Malloc returned NULL in %s", fun_name);
        return NULL;
    }

    return p;
}