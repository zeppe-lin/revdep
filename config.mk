# project metadata
NAME        = revdep
VERSION     = 4.0
DIST        = ${NAME}-${VERSION}

# paths
PREFIX      = /usr
MANPREFIX   = ${PREFIX}/share/man
BASHCOMPDIR = ${PREFIX}/share/bash-completion/completions

# flags
CPPFLAGS    = -DVERSION=\"${VERSION}\"
CXXFLAGS    = -std=c++11 -pedantic -Wall -Wextra -Wconversion \
	      -Wcast-align -Wunused -Wshadow -Wold-style-cast
LDFLAGS     = -static $(shell pkg-config --libs --static libelf)
