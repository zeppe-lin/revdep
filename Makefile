NAME = prt-utils
VERSION = 0.9.8

TOOLS 	= prtcreate prtrej prtsweep prtcheck prtwash pkgexport pkgsize \
	  prtorphan prtcheckmissing oldfiles finddeps dllist \
	  findredundantdeps pkg_installed portspage pkgfoster \
	  prtverify \
	  revdep/revdep

PREFIX	= /usr
MANDIR	= $(PREFIX)/man
BINDIR	= $(PREFIX)/bin
LIBDIR  = $(PREFIX)/lib
CONFDIR	= /etc

all:
	@echo "Use 'make install' to install prt-utils"

install-man:
	if [ ! -d $(DESTDIR)$(MANDIR)/man1 ]; then \
	  install -d $(DESTDIR)$(MANDIR)/man1; \
	fi
	for manpage in $(TOOLS) prt-utils; do \
	  if [ -f $$manpage.1 ]; then \
	    install -m 644 $$manpage.1 $(DESTDIR)$(MANDIR)/man1/; \
	  fi; \
	done

install-bin:
	if [ ! -d $(DESTDIR)$(BINDIR) ]; then \
	  install -d $(DESTDIR)$(BINDIR); \
	fi
	for binary in $(TOOLS); do \
	  install -m 755 $$binary $(DESTDIR)$(BINDIR)/; \
	done

install-conf:
	if [ ! -d $(DESTDIR)$(CONFDIR) ]; then \
	  install -d $(DESTDIR)$(CONFDIR); \
	fi
	for tool in $(TOOLS); do \
	  if [ -f $$tool.conf ]; then \
	    install -m 644 $$tool.conf $(DESTDIR)$(CONFDIR)/; \
	  fi; \
	done

install-lib:
	for tool in $(TOOLS); do \
	  if [ -d lib/$$tool ]; then \
	    install -d $(DESTDIR)$(LIBDIR)/$$tool; \
	    install -m 644 lib/$$tool/* $(DESTDIR)$(LIBDIR)/$$tool; \
	  fi; \
	done

prtverify:
	sed "s|@@LIBDIR@@|$(LIBDIR)|" prtverify.in $< > prtverify

revdep/revdep:
	@make -C revdep

install: prtverify revdep/revdep install-man install-bin install-lib # install-conf

clean:
	@rm -f prtverify
	@make -C revdep clean

dist: clean
	@rm -rf ${NAME}-${VERSION}
	@mkdir .${NAME}-${VERSION}
	@cp -r * .${NAME}-${VERSION}
	@mv .${NAME}-${VERSION} ${NAME}-${VERSION}
	@tar cJf ${NAME}-${VERSION}.tar.xz ${NAME}-${VERSION}
	@rm -rf ${NAME}-${VERSION}
