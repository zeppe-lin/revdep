.POSIX:

include ../config.mk

all:

install:
	mkdir -p $(DESTDIR)$(BASHCOMPDIR)
	cp -f bash_completion $(DESTDIR)$(BASHCOMPDIR)/revdep

uninstall:
	rm -f $(DESTDIR)$(BASHCOMPDIR)/revdep

clean:

.PHONY: all install uninstall clean
