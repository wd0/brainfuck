CC = clang
SRC = brainfuck.c
NAME = ./brainfuck
CFLAGS = -g -Wall -Wpedantic -Wextra


all:	
	${CC} -o ${NAME} ${CFLAGS} ${SRC} 

clean:
	rm -f ${NAME}
