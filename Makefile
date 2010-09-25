CC=g++
CFLAGS=-Wall -g

program: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm *.o
      
