CFLAGS=-Wall -Wextra -pedantic -std=c99 -O2 -g
LDFLAGS=-lm

all: bin/kimg bin/kimg.1

bin/kimg:main.o
	mkdir -p bin/
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

bin/kimg.1:kimg.1.scdoc
	scdoc < kimg.1.scdoc > bin/kimg.1

DESTDIR=/usr/local
BINDIR=$(DESTDIR)/bin/
MANDIR=$(DESTDIR)/share/man/

install: bin/kimg
	mkdir -p $(BINDIR)
	cp bin/kimg $(BINDIR)

install_man: bin/kimg.1
	mkdir -p $(MANDIR)/man1/
	cp bin/kimg.1 $(MANDIR)/man1/

uninstall:
	$(RM) $(BINDIR)/kimg $(MANDIR)/man1/kimg.1

clean:
	$(RM) bin *.o -rv
