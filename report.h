#ifndef REPORT_H
#define REPORT_H

#include <stddef.h>

/* Attempt to call malloc.  Fail when returns NULL */
void *malloc_or_fail(size_t bytes, const char *fun_name);

#endif /* REPORT_H */