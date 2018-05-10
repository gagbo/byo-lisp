SOURCES = main.c \
          mpc.c
OBJECTS = main.o \
          mpc.o

CFLAGS = -Wall -std=gnu11
LDFLAGS = -ledit

CC = gcc

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

mpc.o: mpc.c
	$(CC) $(CFLAGS) -c mpc.c

lisp: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o lisp

run: lisp
	./lisp

clean:
	rm $(OBJECTS)
