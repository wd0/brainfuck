#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

/* Prints a warning plus a newline and then exits. */
/* A warning ending in a colon will cause warn to append strerror() to the output. */
/* If fmt is NULL then warn returns 1; else it returns 0. */
int
warn(const char *fmt, ...) {
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
void
die(const char *fmt, ...) {
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
void *
emalloc(size_t n) {
    void *m = malloc(n);
    if (!m)
        die("emalloc(%z):", n);
    return m;
}

enum {STACKSIZE = 30000};
enum {EXEC_MEMERR = -4, BF_ERR = -3, EXEC_ILL = -2, EXEC_DONE = -1};

/* Program flow: */
/* load: A program is read completely in from a file. */
/* run: all of a program's instructions are executed. */
/* execute: A single instruction is executed, modifying the stack pointer,
   the stack, or the program counter. */

char *
load(FILE *fin) {
    enum {DEFAULT_TEXTSIZE = BUFSIZ};
    char *text = emalloc(DEFAULT_TEXTSIZE);
    size_t bufsize = DEFAULT_TEXTSIZE; 
    size_t i;
    int c;

    for (i = 0; (c = fgetc(fin)) != EOF; ++i) {
        if (bufsize - 2 <= i) { 
            bufsize *= 2;
            text = realloc(text, bufsize);
        }
        text[i] = c;
    }
    text[i] = '\0';

    return text;
}

char
peek(const char *sp) {
    return *sp;
}

/* jmp's conditional function pointers */
typedef int (*Jmpcond)(int n);

int
unconditional(int n) {
    return (n = 1);
}

int
iszero(int n) {
    return n == 0;
}

int
isnonzero(int n) {
    return n != 0;
}
/* End function pointers */

int
jmp(const char *text, int *pc, char *sp, Jmpcond jmpcond) {
    int offset = 1; /* Start looking at the next instruction. */
    int nesting = 0;
    int status = 0;
    int i;

    if (!jmpcond(peek(sp)))
        return status;

    switch (text[*pc]) {
        /* Jump forwards to matching ']'. */
        case '[':
            for (i = *pc + 1; text[i];  ++i, ++offset) {
                if (text[i] == ']' && nesting == 0) {
                    *pc += offset;
                    break;
                } else if (text[i] == ']') {
                    --nesting;
                } else if (text[i] == '[') {
                    ++nesting;
                } 
            }
            if (text[i] == '\0')
                status = EXEC_ILL; /* No closing ']' */
            break; 

            /* Jump backwards to matching '['. */
        case ']':
            for (i = *pc - 1; i >= 0; --i, ++offset) {
                if (text[i] == '[' && nesting == 0) {
                    *pc -= offset;
                    break;
                } else if (text[i] == '[') {
                    --nesting;
                } else if (text[i] == ']') {
                    ++nesting;
                } 

            }
            if (i < 0)
                status = EXEC_ILL; /* No opening '[' */
            break;

        default: 
            warn("jmp(%c): not a legal jmp call", text[*pc]);
            status = BF_ERR; 
            break;
    }

    return status;
}

int
execute(const char *text, const char *stack, char **sp, int *pc) {
    const char op = text[*pc];
    int status = 0;
    /* Operators */
    if (op == '>') {
        ++*sp;
        if ((long)(*sp - stack) + 1 > STACKSIZE) 
            status = EXEC_MEMERR;
    } else if (op == '<') {
        --*sp;
        if (*sp < stack) 
            status = EXEC_MEMERR;
    } else if (op == '+') {
        ++**sp;
    } else if (op == '-') {
        --**sp;
    } else if (op == '.') {
        putchar(**sp);
    } else if (op == ',') {
        **sp = getchar();
    } else if (op == '[') {
        status = jmp(text, pc, *sp, iszero);
    } else if (op == ']') {
        status = jmp(text, pc, *sp, isnonzero);
    } else if (op == '\0') {
        status = EXEC_DONE;
    }
    return status;
}

int
run(const char *text, const char *filename) {
    char stack[STACKSIZE] = {0};
    char *sp = stack;
    int pc;
    int status = 0; 

    for (pc = 0; status != EXEC_DONE; ++pc) {
        status = execute(text, stack, &sp, &pc);
        if (status <= -2) {
            if (status == EXEC_ILL) {
                warn("%s: text[%d] `%c': illegal instruction", filename, pc, text[pc]);
                break;
            } else if (status == BF_ERR) {
                warn("%s: text[%d]: brainfuck interpreter error", filename, pc);
                break;
            } else if (status == EXEC_MEMERR) {
                warn("%s: text[%d] `%c': stack[%ld] out of bounds", 
                        filename, pc, text[pc], (long)(sp - stack));
                break;
            }
        }
    }

    return status;
}

int
brainfuck(FILE *fin, const char *filename) {
    int status = 0;
    char *text = load(fin);
    if (fclose(fin) || run(text, filename))
        status = 1;
    if (text)
        free(text);
    return status;
}

int
main(int argc, char **argv) {
    const char *filename;
    FILE *fin;
    int status = 0;
    if (argc > 1) {
        while (--argc) {
            filename = *++argv;
            if ((fin = fopen(filename, "r")) == NULL) {
                warn("couldn't open %s:", filename);
                status = 1;
                continue;
            }
            status |= brainfuck(fin, filename);
        }
    } else {
        status |= brainfuck(stdin, "<stdin>");
    }
    return status;
}
