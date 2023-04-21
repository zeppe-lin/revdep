# project metadata
NAME = revdep
VERSION = 2.1
DIST = ${NAME}-${VERSION}

# paths
PREFIX = /usr
MANPREFIX = ${PREFIX}/share/man

# flags
CPPFLAGS = -DNAME=\"${NAME}\" -DVERSION=\"${VERSION}\"
CXXFLAGS = -std=c++11 -pedantic -Wall -Wextra -Wconversion -Wcast-align \
	   -Wunused -Wshadow -Wold-style-cast
LDFLAGS  = -static $(shell pkg-config --libs --static libelf)

# compiler and linker
CXX = c++
LD  = ${CXX}
