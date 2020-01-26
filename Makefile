# author: Diego Krupitza 11808206
# programms: forksort
CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
  
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
forksortOBJECTS = forksort.o
.PHONY: all clean cleanMac abgabe

all: clean forksort

forksort: $(forksortOBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forksort.o: forksort.c

clean:
	rm -rf *.o forksort

cleanMac:
	rm -rf *.o forksort
	rm -rf ._forksort_td.pdf ./._Aufgabe2

abgabe:
	rm -rf Aufgabe2.tgz
	mkdir Aufgabe2_11808206
	cp 1.txt babynames.txt forksort.c Makefile Aufgabe2_11808206/
	tar -cvzf Aufgabe2.tgz Aufgabe2_11808206
	rm -rf Aufgabe2_11808206/
	mv Aufgabe2.tgz ../Aufgabe2.tgz