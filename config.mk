# revdep version
VERSION = 2.0

# paths
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man
ETCDIR = /etc

# flags
CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic
CPPFLAGS += -DVERSION=\"$(VERSION)\"
LDFLAGS += --static $(shell pkg-config --libs --static libelf)

# compiler and linker
CXX ?= g++
LD = $(CXX)
