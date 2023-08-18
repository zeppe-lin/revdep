.POSIX:

include config.mk

OBJS = $(subst   .cpp,.o,$(wildcard src/*.cpp))
MAN1 = $(subst .1.pod,.1,$(wildcard pod/*.1.pod))

all: revdep manpage
manpage: ${MAN1}

%: %.pod
	pod2man -r "${NAME} ${VERSION}" -c ' ' -n $(basename $@) \
		-s $(subst .,,$(suffix $@)) $< > $@

${OBJS}: copyright.h

copyright.h: ${CURDIR}/COPYRIGHT ${CURDIR}/COPYING.BANNER
	{ echo "#ifndef COPYRIGHT_H"            ; \
	  echo "#define COPYRIGHT_H"            ; \
	  echo "#define COPYRIGHT_MESSAGE \\"   ; \
	  sed 's/^.*/"&\\n"/;$$ ! s/$$/ \\/' $^ ; \
	  echo "#endif"                         ; } > $@

.cpp.o:
	${CXX} -c ${CXXFLAGS} ${CPPFLAGS} $< -o $@

revdep: ${OBJS}
	${LD} $^ ${LDFLAGS} -o $@

install: all
	mkdir -p       ${DESTDIR}${PREFIX}/bin
	mkdir -p       ${DESTDIR}${MANPREFIX}/man1
	cp -f revdep   ${DESTDIR}${PREFIX}/bin/
	cp -f ${MAN1}  ${DESTDIR}${MANPREFIX}/man1/
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
	rm -f ${OBJS} copyright.h revdep ${MAN1}
	rm -f ${DIST}.tar.gz

dist: clean
	git archive --format=tar.gz -o ${DIST}.tar.gz --prefix=${DIST}/ HEAD

.PHONY: all install uninstall install-bashcomp uninstall-bashcomp clean dist
