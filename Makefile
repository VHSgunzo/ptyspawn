PROG = ptyspawn
OBJS = main.o error.o
CFLAGS = -g -Wall -static -s -Os -pipe
CC = $(shell which musl-gcc 2>/dev/null||echo cc)

all: ptyspawn

ptyspawn: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

clean:
	rm -vf $(PROG) *.o

install: all
	install -vDm755 $(PROG) "/usr/bin/$(PROG)"

uninstall:
	rm -vf /usr/bin/$(PROG)
