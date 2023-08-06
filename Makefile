CFLAGS?=-W -Wall -Wextra -O2
CFLAGS+=$(shell gfxprim-config --cflags)
LDLIBS=-lm $(shell gfxprim-config --libs-widgets --libs)
BIN=elecalc
DEP=$(BIN:=.dep)

all: $(DEP) $(BIN)

%.dep: %.c
	$(CC) $(CFLAGS) -M $< -o $@

$(BIN): libelec.o ohm_law.o

-include $(DEP)

install:
	install -m 644 -D layout.json $(DESTDIR)/etc/gp_apps/$(BIN)/layout.json
	install -D $(BIN) -t $(DESTDIR)/usr/bin/
	install -D -m 744 $(BIN).desktop -t $(DESTDIR)/usr/share/applications/
	install -D -m 644 $(BIN).png -t $(DESTDIR)/usr/share/$(BIN)/
clean:
	rm -f $(BIN) *.dep *.o
