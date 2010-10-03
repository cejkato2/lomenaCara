CC=g++
CFLAGS=-Wall -g
RED="\033[0;31m"
GREEN="\033[0;32m"
NORMAL="\033[m"
OK=${GREEN}OK${NORMAL}
FAILED=${RED}FAILED${NORMAL}


all: program test test_dqueue

program: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm a.out testdqueue

test: .
	echo Test for error
	echo Test1
	./a.out input-err1.dat 2> /dev/null && echo -e ${FAILED} || echo -e ${OK}
	echo Test2
	./a.out input-err2.dat 2> /dev/null && echo -e ${FAILED} || echo -e ${OK}
	echo Test of alg
	./test.sh
	

test_dqueue: test_dqueue.cpp dqueue.h
	$(CC) $(CFLAGS) -DDEBUG test_dqueue.cpp -o testdqueue
	./testdqueue

pack:
	tar -czf `date "+%F"`snapshot.tar.gz *.cpp *.h *.dat Makefile
      
plot:
	if [ -e vystup1.dat -a -e vystup2.dat ]; then \
	  gnuplot gnuplot.plot; \
	fi;

