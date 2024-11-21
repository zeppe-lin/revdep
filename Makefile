include config.mk
ifeq (${DEBUG},yes)
include debug.mk
endif

OBJS = src/elf-cache.o src/elf.o src/main.o src/pkg.o src/utility.o

all: revdep

revdep: ${OBJS}
	${CXX} $^ ${LDFLAGS} -o $@

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	mkdir -p ${DESTDIR}${MANPREFIX}/man5
	cp -f revdep ${DESTDIR}${PREFIX}/bin/
	cp -f man/revdep.1 ${DESTDIR}${MANPREFIX}/man1/
	cp -f man/revdep.d.5 ${DESTDIR}${MANPREFIX}/man5/
	chmod 0755 ${DESTDIR}${PREFIX}/bin/revdep
	chmod 0644 ${DESTDIR}${MANPREFIX}/man1/revdep.1
	chmod 0644 ${DESTDIR}${MANPREFIX}/man5/revdep.d.5

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/revdep
	rm -f ${DESTDIR}${MANPREFIX}/man1/revdep.1
	rm -f ${DESTDIR}${MANPREFIX}/man5/revdep.d.5

install_bashcomp:
	mkdir -p ${DESTDIR}${BASHCOMPDIR}
	cp -f bash_completion ${DESTDIR}${BASHCOMPDIR}/revdep

uninstall_bashcomp:
	rm -f ${DESTDIR}${BASHCOMPDIR}/revdep

clean:
	rm -f ${OBJS} revdep
	rm -f ${DIST}.tar.gz

dist: clean
	git archive --format=tar.gz -o ${DIST}.tar.gz --prefix=${DIST}/ HEAD

.PHONY: all install uninstall install_bashcomp uninstall_bashcomp clean dist
