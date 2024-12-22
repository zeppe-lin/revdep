# project metadata
NAME        = revdep
VERSION     = 4.0

# paths
PREFIX      = /usr
MANPREFIX   = $(PREFIX)/share/man
BASHCOMPDIR = $(PREFIX)/share/bash-completion/completions

# flags
CPPFLAGS    = -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 \
              -DVERSION=\"$(VERSION)\"
CXXFLAGS    = -std=c++0x -pedantic -Wall -Wextra
LDFLAGS     = -lelf

# compiler and linker
CXX         = c++
LD          = $(CXX)
