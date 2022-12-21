.POSIX:

include config.mk

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: revdep revdep.1

revdep.1: revdep.1.pod
	sed "s|@SYSCONFDIR@|${SYSCONFDIR}|g" $< | pod2man --nourls \
		-r ${VERSION} -c ' ' -n revdep -s 1 - > $@

.cpp.o:
	${CXX} -c ${CXXFLAGS} ${CPPFLAGS} $<

revdep: ${OBJS}
	${LD} $^ ${LDFLAGS} -o $@

check:
	@podchecker *.pod
	@grep -Eiho "https?://[^\"\\'> ]+" *.* | httpx -silent -fc 200 -sc

install: all
	mkdir -p       ${DESTDIR}${PREFIX}/bin
	mkdir -p       ${DESTDIR}${MANPREFIX}/man1
	cp -f revdep   ${DESTDIR}${PREFIX}/bin/
	cp -f revdep.1 ${DESTDIR}${MANPREFIX}/man1/

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/revdep
	rm -f ${DESTDIR}${MANPREFIX}/man1/revdep.1

clean:
	rm -f ${OBJS} revdep revdep.1

.PHONY: all install uninstall clean
