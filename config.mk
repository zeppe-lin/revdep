# See COPYING and COPYRIGHT files for corresponding information.

# revdep version if undefined
VERSION = 2.0

# paths
PREFIX = /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/share/man
ETCDIR = /etc

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
LDFLAGS  = --static $(shell pkg-config --libs --static libelf)

# compiler and linker
CXX = c++
LD  = ${CXX}

# vim:sw=2:ts=2:sts=2:et:cc=72:tw=70
# End of file.
