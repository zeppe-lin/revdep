.POSIX:

include config.mk

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: revdep revdep.1

revdep.1: revdep.1.pod
	pod2man --nourls -r ${VERSION} -c ' ' -n revdep -s 1 $< > $@

.cpp.o:
	${CXX} -c ${CXXFLAGS} ${CPPFLAGS} $<

revdep: ${OBJS}
	${LD} $^ ${LDFLAGS} -o $@

check:
	@echo "=======> Check PODs for errors"
	@podchecker *.pod
	@echo "=======> Check URLs for response code"
	@grep -Eiho "https?://[^\"\\'> ]+" *.* | xargs -P10 -I{} \
		curl -o /dev/null -sw "[%{http_code}] %{url}\n" '{}'

install: all
	mkdir -p       ${DESTDIR}/usr/bin
	mkdir -p       ${DESTDIR}/usr/share/man/man1
	cp -f revdep   ${DESTDIR}/usr/bin/
	cp -f revdep.1 ${DESTDIR}/usr/share/man/man1/
	chmod 0755     ${DESTDIR}/usr/bin/revdep
	chmod 0644     ${DESTDIR}/usr/share/man/man1/revdep.1

uninstall:
	rm -f ${DESTDIR}/usr/bin/revdep
	rm -f ${DESTDIR}/usr/share/man/man1/revdep.1

clean:
	rm -f ${OBJS} revdep revdep.1

.PHONY: all install uninstall clean
