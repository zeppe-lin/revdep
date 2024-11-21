# debug flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CXXFLAGS = -std=c++11 -ggdb3 -fno-omit-frame-pointer -fsanitize=address \
           -fsanitize=leak -fsanitize=undefined -fsanitize-recover=address
LDFLAGS  = $(shell pkg-config --libs libelf) ${CXXFLAGS} -lasan -lubsan
