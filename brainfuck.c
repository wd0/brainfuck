#include <stdio.h>
#include <stdlib.h>
#include "err/err.h"

enum { STACKSIZE = 30000 };
enum { EXEC_MEMERR = -4, BF_ERR = -3, EXEC_ILL = -2, EXEC_DONE = -1 };

/* Program flow: */
/* load: A program is read completely in from a file. */
/* run: all of a program's instructions are executed. */
/* execute: A single instruction is executed, modifying the stack pointer,
   the stack, or the program counter. */

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

char
peek(char *sp) {
    return *sp;
}

/* Function pointers */
typedef int (*Fcond)(int n);

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
jmp(const char *text, int *pc, char *sp, Fcond jmpcond) {
    int offset = 0;
    int nesting = 0;
    int status = 0;
    int i;

    if (!jmpcond(peek(sp)))
	return status;

    switch (text[*pc]) {
	/* Jump forwards to matching ']'. */
	case '[':
	    for (i = *pc + 1; text[i];  ++i) {
		if (text[i] == ']' && nesting == 0) {
		    offset = (i - *pc - 1) - 1; 
		    *pc += offset;
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
	    for (i = *pc - 1; i >= 0; --i) {
		if (text[i] == '[' && nesting == 0) {
		    offset = -(*pc - i - 1) - 1; 
		    *pc += offset;
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

    switch (op) {
	/* Operators */
	case '>':
	    ++*sp;
	    if ((long)(*sp - stack) + 1 > STACKSIZE) {
		status = EXEC_MEMERR;
		--*sp;
	    }
	    break;
	case '<':
	    --*sp;
	    if (*sp < stack) {
		status = EXEC_MEMERR;
		++*sp;
	    }
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
	    status = jmp(text, pc, *sp, iszero);
	    break;
	case ']':
	    status = jmp(text, pc, *sp, isnonzero);
	    break;

	    /* Special */
	case '\0':
	    status = EXEC_DONE;
	    break;

	default: 
	    break;
    }

    return status;
}

int
run(const char *text, const char *name) {
    char stack[STACKSIZE] = { 0 };
    char *sp = stack;
    int pc;
    int status = 0; 

    for (pc = 0; status != EXEC_DONE; ++pc) {
	status = execute(text, stack, &sp, &pc);
	if (status <= -2) {
	    if (status == EXEC_ILL) {
		warn("%s: text[%d] `%c': illegal instruction", name, pc, text[pc]);
		break;
	    } else if (status == BF_ERR) {
		warn("%s: text[%d]: brainfuck interpreter error", name, pc);
		break;
	    } else if (pc == EXEC_MEMERR) {
		warn("%s: text[%d] `%c': stack[%ld] out of bounds", 
			name, pc, text[pc], (long)(sp - stack));
		break;
	    }
	}
    }

    status = pc;
    return status;
}

int
main(int argc, char **argv) {
    const char *filename;
    const char *text;
    FILE *fin;
    int status = 0;
    int ret = 0;

    if (argc > 1) {
	while (--argc) {
	    filename = *++argv;
	    if ((fin = fopen(filename, "r")) == NULL) {
		warn("couldn't open %s:", filename);
		status = 1;
		continue;
	    }
	    text = load(fin);
	    fclose(fin);
	    ret = run(text, filename);
	    if (ret != 0) 
		status = 1;
	    if (text)
		free((void *)text);
	}
    } else {
	text = load(stdin);
	filename = "stdin";
	ret = run(text, filename);
	if (ret != 0) 
	    status = 1;
	if (text) 
	    free((void *)text);
    }

    return status;
}
