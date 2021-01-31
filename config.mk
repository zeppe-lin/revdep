NAME = revdep
VERSION = 2.0

BINDIR = /usr/bin
MANDIR = /usr/share/man/man1
ETCDIR = /etc

CXX ?= g++
LD = $(CXX)

CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic

CPPFLAGS += -DVERSION=\"$(VERSION)\"

LDFLAGS += --static $(shell pkg-config --libs --static libelf)

# End of file
