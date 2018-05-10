SOURCES = parsing.c \
          mpc.c \
          evaluation.c
OBJECTS = parsing.o \
          mpc.o \
          evaluation.o

CFLAGS = -Wall -std=gnu11
LDFLAGS = -ledit -lm

CC = gcc

parsing.o: parsing.c
	$(CC) $(CFLAGS) -c parsing.c

mpc.o: mpc.c
	$(CC) $(CFLAGS) -c mpc.c

evaluation.o: evaluation.c
	$(CC) $(CFLAGS) -c evaluation.c

lisp: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o lisp

run: lisp
	./lisp

clean:
	rm $(OBJECTS)
