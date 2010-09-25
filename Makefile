CC=g++
CFLAGS=-Wall -g

program: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm a.out testdqueue

test:
	./a.out input.dat

test_dqueue: test_dqueue.cpp dqueue.h
	$(CC) $(CFLAGS) -DDEBUG test_dqueue.cpp -o testdqueue
	./testdqueue
      
