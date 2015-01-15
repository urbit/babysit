include config.mk

SRC = babysit.c
OBJ = ${SRC:.c=.o}
CORE = .MAKEFILE-VERSION

all: options babysit

options:
	@echo babysit options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.MAKEFILE-VERSION: Makefile config.mk
	@echo Makefile update.
	@touch .MAKEFILE-VERSION

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.h ${CORE}

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

babysit: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f babysit ${OBJ}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f babysit ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/babysit

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/babysit

.PHONY: all options clean install uninstall
