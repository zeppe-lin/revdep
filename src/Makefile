.POSIX:
.SUFFIXES: .cpp .o

include ../config.mk
#include ../extra/flags-extra.mk
#include ../extra/flags-sanitizer.mk

OBJS = elf_cache.o elf.o main.o pkg.o utility.o
BIN1 = revdep

all: $(BIN1)

.cpp.o:
	$(CXX) -c $< $(CPPFLAGS) $(CXXFLAGS)

$(BIN1): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN1) $(DESTDIR)$(PREFIX)/bin
	cd $(DESTDIR)$(PREFIX)/bin && chmod 0755 $(BIN1)

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN1)

clean:
	rm -f $(OBJS) $(BIN1)

.PHONY: all install uninstall clean
