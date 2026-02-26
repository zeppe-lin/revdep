.POSIX:
.SUFFIXES: .cpp .o

include ../../config.mk
#include ../../extra/flags-extra.mk
#include ../../extra/flags-sanitizer.mk

LIB_NAME = librevdep.a

LIB_SRC = \
	elf_cache.cpp \
	elf.cpp \
	pkg.cpp \
	utility.cpp \
	revdep_engine.cpp \
	revdep_parallel.cpp \
	revdep_format.cpp

LIB_HDR = \
	elf_cache.h \
	elf.h \
	pkg.h \
	utility.h \
	revdep_context.h \
	revdep_engine.h \
	revdep_parallel.h \
	revdep_format.h \
	librevdep.h

LIB_OBJ = $(LIB_SRC:.cpp=.o)

.cpp.o:
	$(CXX) -c $< $(CPPFLAGS) $(CXXFLAGS)

all: $(LIB_NAME)

$(LIB_NAME): $(LIB_OBJ)
	ar rcs $@ $^

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/include/librevdep
	cp -f $(LIB_NAME) $(DESTDIR)$(PREFIX)/lib
	cp -f $(LIB_HDR) $(DESTDIR)$(PREFIX)/include/librevdep

uninstall:
	cd $(DESTDIR)$(PREFIX)/lib && rm -f $(LIB_NAME)
	cd $(DESTDIR)$(PREFIX)/include/librevdep && rm -f $(LIB_HDR)

clean:
	rm -f $(LIB_OBJ) $(LIB_NAME)

.PHONY: all install uninstall clean
