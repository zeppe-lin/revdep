include config.mk

OBJS = $(subst .cpp,.o,$(wildcard *.cpp))

all: revdep revdep.1

revdep.1:
	pod2man -r "${NAME} ${VERSION}" -c ' ' -n revdep -s 1 revdep.1.pod > $@

revdep: ${OBJS}
	${LD} $^ ${LDFLAGS} -o $@

install: all
	mkdir -p       ${DESTDIR}${PREFIX}/bin
	mkdir -p       ${DESTDIR}${MANPREFIX}/man1
	cp -f revdep   ${DESTDIR}${PREFIX}/bin/
	cp -f revdep.1 ${DESTDIR}${MANPREFIX}/man1/
	chmod 0755     ${DESTDIR}${PREFIX}/bin/revdep
	chmod 0644     ${DESTDIR}${MANPREFIX}/man1/revdep.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/revdep
	rm -f ${DESTDIR}${MANPREFIX}/man1/revdep.1

install-bashcomp:
	mkdir -p ${DESTDIR}${BASHCOMPDIR}
	cp -f bash_completion ${DESTDIR}${BASHCOMPDIR}/revdep

uninstall-bashcomp:
	rm -f ${DESTDIR}${BASHCOMPDIR}/revdep

clean:
	rm -f ${OBJS} revdep revdep.1
	rm -f ${DIST}.tar.gz

dist: clean
	git archive --format=tar.gz -o ${DIST}.tar.gz --prefix=${DIST}/ HEAD

.PHONY: all install uninstall install-bashcomp uninstall-bashcomp clean dist
