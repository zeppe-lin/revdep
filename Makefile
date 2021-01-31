.SUFFIXES: .cpp .o
include config.mk

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)
BIN = revdep
MAN = revdep.1

all: $(BIN) $(MAN)

%: %.in
	sed -e "s/#VERSION#/$(VERSION)/" $< > $@

.cpp.o:
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(CPPFLAGS)

$(BIN): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS)

install: all
	install -d $(DESTDIR)$(ETCDIR)/revdep.d
	install -Dm0755 $(BIN) $(DESTDIR)$(BINDIR)
	install -Dm0644 $(MAN) $(DESTDIR)$(MANDIR)

uninstall:
	rm -rf $(DESTDIR)$(ETCDIR)/revdep.d
	rm -f  $(DESTDIR)$(BINDIR)/$(BIN)
	rm -f  $(DESTDIR)$(MANDIR)/$(MAN)

clean:
	rm $(OBJ) $(BIN) $(MAN)

.PHONY: all install uninstall clean
