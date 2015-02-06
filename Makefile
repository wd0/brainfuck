CC = cc
SRC = brainfuck.c err.c
NAME = brainfuck
CFLAGS = -Wall -Wpedantic -Wextra


all:	
	${CC} -o ${NAME} ${CFLAGS} ${SRC} 

test: all
	./${NAME} test/*.bf

clean:
	rm -f ${NAME}
