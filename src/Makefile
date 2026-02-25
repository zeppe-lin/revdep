.POSIX:
.SUFFIXES: .cpp .o

include ../config.mk
#include ../extra/flags-extra.mk
#include ../extra/flags-sanitizer.mk

OBJS = revdep.o
BIN1 = revdep

LIB_DIR = ./librevdep
LIB_NAME = $(LIB_DIR)/librevdep.a
LIB_CPPFLAGS = -I$(LIB_DIR)

all: $(BIN1)

.cpp.o:
	$(CXX) -c $< $(LIB_CPPFLAGS) $(CPPFLAGS) $(CXXFLAGS)

$(LIB_NAME):
	cd $(LIB_DIR) && $(MAKE)

$(BIN1): $(OBJS) $(LIB_NAME)
	$(CXX) $^ $(LDFLAGS) -o $@

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN1) $(DESTDIR)$(PREFIX)/bin
	cd $(DESTDIR)$(PREFIX)/bin && chmod 0755 $(BIN1)

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN1)

clean:
	rm -f $(OBJS) $(BIN1)
	cd $(LIB_DIR) && $(MAKE) clean

.PHONY: all install uninstall clean
