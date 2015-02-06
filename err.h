/* err.h */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

/* Prints a warning plus a newline and then exits. */
/* A warning ending in a colon will cause `die' to append strerror() to the output. */
void die(char *fmt, ...);
/* Warn and then exit. */
void warn(char *fmt, ...);
/* Malloc that never fails. Shows a warning and exits. */
void *emalloc(size_t n);
