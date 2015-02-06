#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

/* Prints a warning plus a newline and then exits. */
/* A warning ending in a colon will cause warn to append strerror() to the output. */
/* If fmt is NULL then warn returns 1; else it returns 0. */
int warn(char *fmt, ...)
{
    va_list ap;
    int len;

    if (fmt == NULL) {
	warn("%s", "warn: attempted to warn with null print format");
	return 1;
    }

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    len = strlen(fmt);
    if (fmt[len-1] == ':') 
	fprintf(stderr, " %s", strerror(errno));
    putchar('\n');

    return 0;
}

/* Warn and then exit. */
void die(char *fmt, ...)
{
    va_list ap;
    int len;

    if (fmt == NULL) {
	fprintf(stderr, "%s", "die: attempted to die with null print format");
	exit(1);
    }

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    len = strlen(fmt);
    if (fmt[len-1] == ':') 
	fprintf(stderr, " %s", strerror(errno));
    putchar('\n');

    exit(1);
}

/* Malloc that never fails. Shows a warning and exits. */
void *emalloc(size_t n)
{
    void *m = malloc(n);

    if (m == NULL)
	die("emalloc(%z):", n);

    return m;
}
