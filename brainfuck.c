#include <stdio.h>
#include <stdlib.h>
#include "err/err.h"

enum { STACKSIZE = 30000 };

/* load: A program is read completely in from a file. */
/* execute: A single instruction is executed, modifying the stack pointer,
   the stack, or the program counter. */
/* run: all of a program's instructions are run. */

const char *
load(FILE *fin) {
    char *program = emalloc(1);
    size_t bufsize = 1;
    size_t i;
    int c;

    for (i = 0; (c = fgetc(fin)) != EOF; ++i) {
	if (bufsize - 1 <= i) { 
	    bufsize *= 2;
	    program = realloc(program, bufsize);
	}
	program[i] = c;
    }

    return program;
}

int
execute(const char *program, char **sp, int pc) {
    const char op = program[pc];
    switch (op) {
	case '>':
	    ++*sp;
	    break;
	case '<':
	    --*sp;
	    break;
	case '+':
	    ++**sp;
	    break;
	case '-':
	    --**sp;
	    break;
	case '.':
	    putchar(**sp);
	    break;
	case ',':
	    **sp = getchar();
	    break;
	case '\0':
	    return 0;
	    break;
	case '\n':
	    break;
	case ' ':
	    break;

	case '[':
	case ']':
	default: 
	    return -1;
    }

    return pc;
}

int
run(const char *program) {
    char base[STACKSIZE] = { 0 };
    char *sp = base;
    int pc;

    for (pc = 0; (pc = execute(program, &sp, pc)); ++pc) {
	if (pc == -1)
	    warn("illegal instruction");
    }

    return pc;
}


int
main(int argc, char **argv) {
    char *filename;
    const char *program;
    FILE *fin;
    int status = 0;

    if (argc > 1) {
	while (--argc > 1) {
	    filename = *++argv;
	    if ((fin = fopen(filename, "r")) == NULL) {
		warn("couldn't open %s:", filename);
		status = 1;
		continue;
	    }
	    program = load(fin);
	    fclose(fin);
	    run(program);
	    if (program)
		free(program);
	}
    } else {
	program = load(stdin);
	run(program);
	if (program) 
	    free(program);
    }

    return status;
}
