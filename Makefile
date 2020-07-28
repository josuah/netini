NAME = netgraph
VERSION = 0.0

SRC = src/map.c src/compat/strip.c src/compat/strchomp.c src/compat/strlcpy.c \
  src/conf.c src/mem.c src/log.c src/ip.c src/arr.c

HDR = src/mem.h src/map.h src/test.h src/compat.h src/log.h src/conf.h src/ip.h \
  src/arr.h

BIN = netgraph-routing

OBJ = ${SRC:.c=.o}

LIB =

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

W = -Wall -Wextra -std=c99 --pedantic
I = -I./src
L =
D = -D_POSIX_C_SOURCE=200811L -DVERSION='"${VERSION}"'
CFLAGS = $I $D $W -g
LDFLAGS = $L -static

all: ${BIN}

.c.o:
	${CC} -c ${CFLAGS} -o $@ $<

${OBJ}: ${HDR}
${BIN}: ${OBJ} ${BIN:=.o}
	${CC} ${LDFLAGS} -o $@ $@.o ${OBJ} ${LIB}

clean:
	rm -rf *.o */*.o ${BIN} ${NAME}-${VERSION} *.gz

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -rf ${BIN} ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -rf doc/*.1 ${DESTDIR}${MANPREFIX}/man1

dist: clean
	mkdir -p ${NAME}-${VERSION}
	cp -r README Makefile doc ${SRC} ${NAME}-${VERSION}
	tar -cf - ${NAME}-${VERSION} | gzip -c >${NAME}-${VERSION}.tar.gz
