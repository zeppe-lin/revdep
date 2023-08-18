# project metadata
NAME         = revdep
VERSION      = 3.1
HOMEPAGE     = https://github.com/zeppe-lin/revdep/
BUGTRACKER   = https://github.com/zeppe-lin/revdep/issues/
DIST         = ${NAME}-${VERSION}

# paths
PREFIX       = /usr
MANPREFIX    = ${PREFIX}/share/man
BASHCOMPDIR  = ${PREFIX}/share/bash-completion/completions

# flags
CPPFLAGS     = -DPROJECT_NAME=\"${NAME}\" \
	       -DPROJECT_VERSION=\"${VERSION}\" \
	       -DPROJECT_HOMEPAGE=\"${HOMEPAGE}\" \
	       -DPROJECT_BUGTRACKER=\"${BUGTRACKER}\"
CXXFLAGS     = -std=c++11 -pedantic -Wall -Wextra -Wconversion \
	       -Wcast-align -Wunused -Wshadow -Wold-style-cast
LDFLAGS      = -static $(shell pkg-config --libs --static libelf)

# compiler and linker
CXX          = c++
LD           = ${CXX}
