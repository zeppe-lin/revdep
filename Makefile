include config.mk
ifeq (${DEBUG},yes)
include debug.mk
endif

OBJS = src/elf-cache.o src/elf.o src/main.o src/pkg.o src/utility.o
BIN1 = revdep
MAN1 = revdep.1
MAN5 = revdep.d.5

all: revdep

revdep: ${OBJS}
	${CXX} $^ ${LDFLAGS} -o src/$@

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	mkdir -p ${DESTDIR}${MANPREFIX}/man5
	cd src && cp -f ${BIN1} ${DESTDIR}${PREFIX}/bin
	cd man && cp -f ${MAN1} ${DESTDIR}${MANPREFIX}/man1
	cd man && cp -f ${MAN5} ${DESTDIR}${MANPREFIX}/man5
	cd ${DESTDIR}${PREFIX}/bin     && chmod 0755 ${BIN1}
	cd ${DESTDIR}${MANPREFIX}/man1 && chmod 0644 ${MAN1}
	cd ${DESTDIR}${MANPREFIX}/man5 && chmod 0644 ${MAN5}

uninstall:
	cd ${DESTDIR}${PREFIX}/bin     && rm -f ${BIN1}
	cd ${DESTDIR}${MANPREFIX}/man1 && rm -f ${MAN1}
	cd ${DESTDIR}${MANPREFIX}/man5 && rm -f ${MAN5}

install_bashcomp:
	mkdir -p ${DESTDIR}${BASHCOMPDIR}
	cp -f extra/bash_completion ${DESTDIR}${BASHCOMPDIR}/revdep

uninstall_bashcomp:
	rm -f ${DESTDIR}${BASHCOMPDIR}/revdep

clean:
	rm -f ${OBJS} src/revdep
	rm -f ${DIST}.tar.gz

dist: clean
	git archive --format=tar.gz -o ${DIST}.tar.gz --prefix=${DIST}/ HEAD

.PHONY: all ${BIN1} install uninstall install_bashcomp uninstall_bashcomp clean dist
