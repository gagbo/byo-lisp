SOURCES = parsing.c \
          mpc.c
OBJECTS = parsing.o \
          mpc.o

CFLAGS = -Wall -std=gnu11
LDFLAGS = -ledit

CC = gcc

parsing.o: parsing.c
	$(CC) $(CFLAGS) -c parsing.c

mpc.o: mpc.c
	$(CC) $(CFLAGS) -c mpc.c

lisp: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o lisp

run: lisp
	./lisp

clean:
	rm $(OBJECTS)
