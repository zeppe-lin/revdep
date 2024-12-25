.POSIX:

include ../config.mk

MAN1 = revdep.1
MAN5 = revdep.d.5

all:

lint:
	mandoc -Tlint $(MAN1) $(MAN5)

install:
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	mkdir -p $(DESTDIR)$(MANPREFIX)/man5
	cp -f $(MAN1) $(DESTDIR)$(MANPREFIX)/man1
	cp -f $(MAN5) $(DESTDIR)$(MANPREFIX)/man5
	cd $(DESTDIR)$(MANPREFIX)/man1 && chmod 0644 $(MAN1)
	cd $(DESTDIR)$(MANPREFIX)/man5 && chmod 0644 $(MAN5)

uninstall:
	cd $(DESTDIR)$(MANPREFIX)/man1 && rm -f $(MAN1)
	cd $(DESTDIR)$(MANPREFIX)/man5 && rm -f $(MAN5)

clean:

.PHONY: all lint install uninstall clean
