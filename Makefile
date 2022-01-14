include config.mk

.POSIX:

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: revdep revdep.1

revdep.1: revdep.1.pod
	pod2man --nourls -r $(VERSION) -c ' ' -n revdep -s 1 $^ > $@

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

revdep: $(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@

install: all
	install -d $(DESTDIR)$(ETCDIR)/revdep.d
	install -m 755 -D -t $(DESTDIR)$(BINDIR)/      revdep
	install -m 644 -D -t $(DESTDIR)$(MANDIR)/man1/ revdep.1

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/revdep
	rm -f $(DESTDIR)$(MANDIR)/man1/revdep.1

clean:
	rm -f $(OBJS) revdep revdep.1

.PHONY: all install uninstall clean
