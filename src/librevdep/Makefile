.POSIX:
.SUFFIXES: .cpp .o

include ../../config.mk
#include ../../extra/flags-extra.mk
#include ../../extra/flags-sanitizer.mk

LIB_NAME = librevdep.a
LIB_SRC = elf_cache.cpp elf.cpp pkg.cpp utility.cpp
LIB_HDR = elf_cache.h   elf.h   pkg.h   utility.h
LIB_OBJ = elf_cache.o   elf.o   pkg.o   utility.o

.cpp.o:
	$(CXX) -c $< $(CPPFLAGS) $(CXXFLAGS)

all: $(LIB_NAME)

$(LIB_NAME): $(LIB_OBJ)
	ar rcs $@ $^

clean:
	rm -f $(LIB_OBJ) $(LIB_NAME)

.PHONY: all clean
