include $(CURDIR)/../config.mk
#include $(CURDIR)/../extra/asan_ubsan.mk
#include $(CURDIR)/../extra/extra_warns.mk

OBJS = elf-cache.o elf.o main.o pkg.o utility.o
BIN1 = revdep

all: $(BIN1)

$(BIN1): $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN1) $(DESTDIR)$(PREFIX)/bin
	chmod 0755 $(DESTDIR)$(PREFIX)/bin/$(BIN1)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN1)

clean:
	rm -f $(OBJS) $(BIN1)

.PHONY: all install uninstall clean
