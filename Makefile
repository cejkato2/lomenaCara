CC=g++
CFLAGS=-Wall -g
RED="\033[0;31m"
GREEN="\033[0;32m"
NORMAL="\033[m"


all: program test test_dqueue

program: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm a.out testdqueue

test:
	./a.out input.dat
	echo Test for error
	./a.out input-err1.dat && echo -e ${RED}FAILED${NORMAL} || echo -e ${GREEN}OK${NORMAL}
	./a.out input-err2.dat && echo -e ${RED}FAILED${NORMAL} || echo -e ${GREEN}OK${NORMAL}

test_dqueue: test_dqueue.cpp dqueue.h
	$(CC) $(CFLAGS) -DDEBUG test_dqueue.cpp -o testdqueue
	./testdqueue

pack:
	tar -czf `date "+%F"`snapshot.tar.gz *.cpp *.h *.dat Makefile
      
