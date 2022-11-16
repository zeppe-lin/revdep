# This file is a part of revdep.
# See COPYING and COPYRIGHT files for corresponding information.

include config.mk

.POSIX:

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: revdep revdep.1

revdep.1: revdep.1.pod
	pod2man --nourls -r ${VERSION} -c ' ' -n revdep -s 1 $^ > $@

.cpp.o:
	${CXX} -c ${CXXFLAGS} ${CPPFLAGS} $< -o $@

revdep: ${OBJS}
	${LD} $^ ${LDFLAGS} -o $@

install: all
	install -d ${DESTDIR}${ETCDIR}/revdep.d
	install -m 0755 -Dt ${DESTDIR}${BINDIR}/      revdep
	install -m 0644 -Dt ${DESTDIR}${MANDIR}/man1/ revdep.1

uninstall:
	rm -f ${DESTDIR}${BINDIR}/revdep
	rm -f ${DESTDIR}${MANDIR}/man1/revdep.1

clean:
	rm -f ${OBJS} revdep revdep.1

.PHONY: all install uninstall clean

# vim:cc=72:tw=70
# End of file.
