NAME = netini
VERSION = 0.4
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

D = -D_POSIX_C_SOURCE=200811L -DVERSION='"${VERSION}"'
CFLAGS = -g -Wall -Wextra -std=c99 --pedantic -fPIC $D
LDFLAGS = -static

SRC = mem.c ip.c log.c strchomp.c strlcpy.c strip.c conf.c array.c netini.c \
  mac.c
HDR = ip.h conf.h array.h test.h compat.h mem.h netini.h mac.h log.h
BIN = netini-dot
OBJ = ${SRC:.c=.o}
MAN1 = ${BIN:=.1}

all: ${BIN}

.c.o:
	${CC} -c ${CFLAGS} -o $@ $<

${OBJ} ${BIN:=.o}: Makefile ${HDR}

${BIN}: ${OBJ} ${BIN:=.o}
	${CC} ${LDFLAGS} -o $@ $@.o ${OBJ} ${LIB}

clean:
	rm -rf *.o ${BIN} ${NAME}-${VERSION} *.tgz

install: ${BIN}
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f bin/* ${BIN} ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f ${MAN1} ${DESTDIR}${MANPREFIX}/man1

dist:
	git archive v${VERSION} --prefix=${NAME}-${VERSION}/ \
	| gzip >${NAME}-$V.tgz

site:
	notmarkdown README.md | notmarkdown-html | cat .head.html -> index.html
	notmarkdown README.md | notmarkdown-gph | cat .head.gph -> index.gph
	mkdir -p man
	mandoc -Thtml -Ofragment ${MAN1} | cat .head.html - >man/index.html
	mandoc -Tutf8 ${MAN1} | ul -t dumb >man/index.gph
	sed -i "s/NAME/${NAME}/g; s/VERSION/${VERSION}/g" index.* man/index.*
