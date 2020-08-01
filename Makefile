NAME = netini
VERSION = 0.2

SRC = src/mem.c src/ip.c src/log.c src/compat/strchomp.c src/compat/strlcpy.c \
  src/compat/strip.c src/conf.c src/array.c src/netini.c src/mac.c

HDR = src/ip.h src/conf.h src/array.h src/test.h src/compat.h src/mem.h \
  src/netini.h src/mac.h src/log.h

BIN = netini-dot

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

${OBJ} ${BIN:=.o}: Makefile ${HDR}
${BIN}: ${OBJ} ${BIN:=.o}
	${CC} ${LDFLAGS} -o $@ $@.o ${OBJ} ${LIB}

clean:
	rm -rf *.o */*.o ${BIN} ${NAME}-${VERSION} *.gz

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -rf bin/* ${BIN} ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -rf doc/*.1 ${DESTDIR}${MANPREFIX}/man1

dist: clean
	mkdir -p ${NAME}-${VERSION}
	cp -r README.md Makefile *.c bin doc src ${NAME}-${VERSION}
	tar -cf - ${NAME}-${VERSION} | gzip -c >${NAME}-${VERSION}.tar.gz
