CC = cc
SRC = brainfuck.c
CFLAGS = -Wall -Wextra -ansi -pedantic 

NAME = brainfuck
DOCNAME = ${NAME}.1.gz

INSTALLPATH = /usr/local
BINPATH = ${INSTALLPATH}/bin
DOCPATH = ${INSTALLPATH}/share/man/man1

all: build doc

build:
	${CC} -o ${NAME} ${CFLAGS} ${SRC} 

test: all
	! ./${NAME} test/*.bf

clean:
	rm -f ${NAME}
	rm -f ${DOCNAME}

doc:
	gzip -c ${NAME}.1 >${DOCNAME}

install: build doc
	install -D ${NAME} ${BINPATH}
	install -D ${DOCNAME} ${DOCPATH}/${DOCNAME} -m 0644

remove:
	rm -f ${BINPATH}/${NAME}
	rm -f ${DOCPATH}/${DOCNAME}
