NAME = netini
VERSION = 0.4
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

CFLAGS = -g -Wall -Wextra \
   -std=c99 --pedantic -D_POSIX_C_SOURCE=200811L -DVERSION='"${VERSION}"'
LDFLAGS = -static

SRC = mem.c ip.c log.c strchomp.c strlcpy.c strip.c conf.c array.c netini.c \
  mac.c
HDR = ip.h conf.h array.h test.h compat.h mem.h netini.h mac.h log.h
BIN = netini-dot
OBJ = ${SRC:.c=.o}
MAN1 = ${BIN:=.1}
LIB =
all: ${BIN}

.c.o:
	${CC} -c ${CFLAGS} -o $@ $<

${OBJ} ${BIN:=.o}: Makefile ${HDR}
${BIN}: ${OBJ} ${BIN:=.o}
	${CC} ${LDFLAGS} -o $@ $@.o ${OBJ} ${LIB}

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -rf bin/* ${BIN} ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -rf ${MAN1} ${DESTDIR}${MANPREFIX}/man1

dist:
	git archive v${VERSION} --prefix=netini-${VERSION}/ \
	  | gzip >netini-${VERSION}.tgz

site: dist
	notmarkdown README.md | notmarkdown-html | cat .head.html -> index.html
	notmarkdown README.md | notmarkdown-gph | cat .head.gph -> index.gph
	sed -i "s/VERSION/${VERSION}/g" index.*

clean:
	rm -rf *.o */*.o ${BIN} ${NAME}-${VERSION} *.tgz

