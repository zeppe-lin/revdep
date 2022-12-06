# See COPYING and COPYRIGHT files for corresponding information.

.POSIX:

include config.mk

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: revdep revdep.1

revdep.1: revdep.1.pod
	pod2man --nourls -r ${VERSION} -c ' ' -n revdep -s 1 $^ > $@

.cpp.o:
	${CXX} -c ${CXXFLAGS} ${CPPFLAGS} $<

revdep: ${OBJS}
	${LD} -o $@ ${LDFLAGS} $^

install: all
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}/man1
	mkdir -p ${DESTDIR}${ETCDIR}/revdep.d
	cp -f revdep   ${DESTDIR}${BINDIR}/
	cp -f revdep.1 ${DESTDIR}${MANDIR}/man1/

uninstall:
	rm -f ${DESTDIR}${BINDIR}/revdep
	rm -f ${DESTDIR}${MANDIR}/man1/revdep.1

clean:
	rm -f ${OBJS} revdep revdep.1

.PHONY: all install uninstall clean

# vim:cc=72:tw=70
# End of file.
