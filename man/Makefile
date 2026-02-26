.POSIX:

include ../config.mk

MAN1 = revdep.1
MAN3 = librevdep.3 revdep_context.3 revdep_engine.3 revdep_format.3 revdep_parallel.3
MAN5 = revdep.d.5
MAN7 = librevdep.7 revdep_semantics.7

all: $(MAN1) $(MAN3) $(MAN5) $(MAN7)

$(MAN1) $(MAN3) $(MAN5) $(MAN7):
	scdoc < $@.scdoc > $@

install: all
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	mkdir -p $(DESTDIR)$(MANPREFIX)/man3
	mkdir -p $(DESTDIR)$(MANPREFIX)/man5
	mkdir -p $(DESTDIR)$(MANPREFIX)/man7
	cp -f $(MAN1) $(DESTDIR)$(MANPREFIX)/man1
	cp -f $(MAN3) $(DESTDIR)$(MANPREFIX)/man3
	cp -f $(MAN5) $(DESTDIR)$(MANPREFIX)/man5
	cp -f $(MAN7) $(DESTDIR)$(MANPREFIX)/man7
	cd $(DESTDIR)$(MANPREFIX)/man1 && chmod 0644 $(MAN1)
	cd $(DESTDIR)$(MANPREFIX)/man3 && chmod 0644 $(MAN3)
	cd $(DESTDIR)$(MANPREFIX)/man5 && chmod 0644 $(MAN5)
	cd $(DESTDIR)$(MANPREFIX)/man7 && chmod 0644 $(MAN7)

uninstall:
	cd $(DESTDIR)$(MANPREFIX)/man1 && rm -f $(MAN1)
	cd $(DESTDIR)$(MANPREFIX)/man3 && rm -f $(MAN3)
	cd $(DESTDIR)$(MANPREFIX)/man5 && rm -f $(MAN5)
	cd $(DESTDIR)$(MANPREFIX)/man7 && rm -f $(MAN7)

clean:
	rm -f $(MAN1) $(MAN3) $(MAN5) $(MAN7)

.PHONY: all install uninstall clean
