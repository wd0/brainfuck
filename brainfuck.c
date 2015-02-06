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
    enum { DEFAULT_TEXTSIZE = 128 };
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

enum { BF_ERR = -3, EXEC_ILL = -2, EXEC_DONE = -1 };

char
peek(char *sp) {
    return *sp;
}

/* Function pointers */
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

typedef struct {
    const char *text;
    char *stack;
    char *sp;
    int pc;
} Prog;

typedef int (*Fcond)(int n);

/* TODO: This currently jumps to the 'next' bracket it finds. It needs to jump
   to the -matching- bracket. */

int
jmp(const char *text, int pc, char *sp, Fcond cond) {
    int offset = 0;
    int nesting = 0;
    int i;

    if (!cond(peek(sp)))
	return pc;

    switch (text[pc]) {
	/* Jump forwards to matching ']'. */
	case '[':
	    for (i = pc + 1; text[i];  ++i) {
		if (text[i] == ']' && nesting == 0) {
		    offset = (i - pc - 1) - 1; 
		    goto got_offset; 
		} else if (text[i] == ']') {
		    --nesting;
		} else if (text[i] == '[') {
		    ++nesting;
		} 
	    }
	    if (text[i] == '\0')
		return EXEC_ILL; /* No closing ']' */
	    break; 

	    /* Jump backwards to matching '['. */
	case ']':
	    for (i = pc - 1; i >= 0; --i) {
		if (text[i] == '[' && nesting == 0) {
		    offset = -(pc - i - 1) - 1; 
		    goto got_offset; 
		} else if (text[i] == '[') {
		    --nesting;
		} else if (text[i] == ']') {
		    ++nesting;
		} 

	    }
	    if (i < 0)
		return EXEC_ILL; /* No opening '[' */
	    break;

	default: 
	    warn("jmp(%c): not a legal jmp call", text[pc]);
	    return BF_ERR; 
	    break;
    }

got_offset:
    return pc + offset;

}

int
execute(const char *text, char **sp, int pc) {
    const char op = text[pc];
    switch (op) {
	/* Ops */
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
	case '[':
	    pc = jmp(text, pc, *sp, iszero);
	    break;
	case ']':
	    pc = jmp(text, pc, *sp, isnonzero);
	    break;

	    /* Special */
	case '\0':
	    return EXEC_DONE;
	    break;

	default: 
	    break;
    }

    return pc;
}

int
run(Prog *p) {
    char base[STACKSIZE] = { 0 };
    p->stack = base;
    p->sp = p->stack;
    int status; 

    for (p->pc = 0; (p->pc = execute(p->text, &p->sp, p->pc)) != EXEC_DONE; ++p->pc) {
	warn("p->sp = %p", p->sp);
	if (p->pc <= -2) {
	    if (p->pc == EXEC_ILL) {
		warn("illegal instruction `%c'", p->text[p->pc]);
		break;
	    } else if (p->pc == BF_ERR) {
		warn("brainfuck interpreter error: instruction `%c' failed", p->text[p->pc]);
		break;
	    }
	}
    }

    status = p->pc;
    return status;
}


int
main(int argc, char **argv) {
    char *filename;
    FILE *fin;
    int status = 0;

    Prog program;
    Prog *p = &program;

    if (argc > 1) {
	while (--argc) {
	    filename = *++argv;
	    if ((fin = fopen(filename, "r")) == NULL) {
		warn("couldn't open %s:", filename);
		status = 1;
		continue;
	    }
	    p->text = load(fin);
	    fclose(fin);
	    run(p);
	    if (p->text)
		free(p->text);
	}
    } else {
	p->text = load(stdin);
	run(p);
	if (p->text) 
	    free(p->text);
    }

    return status;
}
