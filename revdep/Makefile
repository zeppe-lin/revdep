VERSION  := 2.0
BINDIR   := /usr/bin
MANDIR   := /usr/man/man1
ETCDIR   := /etc/revdep.d
CXXFLAGS ?= -O2
CXXFLAGS += -std=gnu++0x -Wall -Wextra -g -DVERSION='"$(VERSION)"'
LDFLAGS  += -static -lelf
SOURCES  := elf.cpp utility.cpp main.cpp pkg.cpp elf-cache.cpp
OBJECTS  := $(patsubst %.cpp,%.o,$(SOURCES))

all: revdep revdep.1

install: all
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(MANDIR)
	install -d $(DESTDIR)$(ETCDIR)
	install -m 0755 revdep $(DESTDIR)$(BINDIR)/
	install -m 0644 revdep.1 $(DESTDIR)$(MANDIR)/

dist: revdep.1
	git archive --format tar --prefix revdep-$(VERSION)/ HEAD | xz > revdep-$(VERSION).tar.xz

revdep: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o revdep

revdep.1: revdep.man
	txt2man revdep.man > revdep.1

clean:
	$(RM) $(OBJECTS) revdep
