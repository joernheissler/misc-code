# DEBUG=-ggdb3 -O0

SONAME=libmydb.so.1
RLNAME=libmydb.so.1.0.1
CFLAGS=-std=c99 -O2 -Wall -Wextra -pedantic -Wshadow -Werror -march=native $(DEBUG)
LFLAGS=-shared -Wl,-soname,$(SONAME) -lmysqlclient
PREFIX=/usr/local
LIBDIR=$(PREFIX)/lib
INCLUDEDIR=$(PREFIX)/include

all: $(RLNAME)

$(RLNAME): mydb.o Makefile
	gcc $(LFLAGS) -o $@ $<

mydb.o: mydb.c mydb.h config.h Makefile
	gcc -c $(CFLAGS) -fpic -o $@ $<

config.h: mysql_options.sh Makefile
	sh $< > $@

install: $(RLNAME) mydb.h
	cp $< $(LIBDIR)/
	cp mydb.h $(INCLUDEDIR)/
	ln -sf $(SONAME) $(LIBDIR)/libmydb.so
	ldconfig -n $(LIBDIR)/

clean:
	rm -f $(RLNAME) mydb.o config.h example example.o

example: example.o $(RLNAME)
	gcc -o $@ $< -L. -lmydb

example.o: example.c Makefile $(RLNAME)
	gcc -c $(CFLAGS) -o $@ $<
