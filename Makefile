SOURCES = main.c
OBJECTS = main.o

CFLAGS = -Wall -std=gnu11
LDFLAGS = -ledit

CC = gcc

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

lisp: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o lisp

run: lisp
	./lisp

clean:
	rm $(OBJECTS)
