#include <stdio.h>

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
}

const char *
execute(const char **sp, const char op) {
    switch (op) {
	case '>':
	    ++*sp;
	case '<':
	    --*sp;
	case '+':
	    ++**sp;
	case '-':
	    --**sp;
	case '.':
	    putchar(**sp);
	case ',':
	    **sp = getchar();
	case '[':
	case ']':
	default:
	    return (const char *) -1;



int
run(const char *program) {
    char base[STACKSIZE] = { 0 };
    char *sp = base;
    const char *pc = program;

    while ((pc = execute(&sp, program[pc])) != -1)
	;

    return pc;
}


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
	    free(program);
	}
    } else {
	program = load(stdin);
	run(program);
    }
    return status;
}
