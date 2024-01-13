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
ifeq (${DEBUG}, 1)
CXXFLAGS    = -ggdb3 -fno-omit-frame-pointer -fsanitize=address \
              -fsanitize=leak -fsanitize=undefined -fsanitize-recover=address
LDFLAGS     = $(shell pkg-config --libs libelf) ${CXXFLAGS} -lasan -lubsan
else
CXXFLAGS    = -std=c++11 -pedantic -Wall -Wextra -Wconversion \
	      -Wcast-align -Wunused -Wshadow -Wold-style-cast
LDFLAGS     = -static $(shell pkg-config --libs --static libelf)
endif
