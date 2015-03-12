VERSION := 1.1
BINDIR  := /usr/bin
MANDIR  := /usr/man/man1
CFLAGS  ?= -O2
CFLAGS  += -Wall -Wextra -Wno-unused-parameter -DVERSION='"$(VERSION)"'
LDFLAGS += -lz -lelf
CC      ?= cc

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: program man

program: revdep

man: revdep.1

revdep: main.o global.o utility.o package.o
	$(CC) $^ $(LDFLAGS) -o $@

revdep.1: revdep.man
	txt2man -t revdep $< > $@

install: revdep revdep.1
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(MANDIR)
	install -m 0755 revdep $(DESTDIR)$(BINDIR)/
	install -m 0644 revdep.1 $(DESTDIR)$(MANDIR)/

dist: man
	tar --transform s:^:revdep-$(VERSION)/: -c *.c *.h *.man *.1 COPYING Makefile | xz > revdep-$(VERSION).tar.xz

clean:
	rm -f *.o revdep revdep.1
