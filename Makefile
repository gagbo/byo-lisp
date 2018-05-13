CC = gcc

CFLAGS = -Wall -std=gnu11 -g
LDFLAGS = -ledit -lm

BUILD_DIR = build
SRC_DIR = src
SOURCES = parsing.c \
          mpc.c \
          evaluation.c \
          lval.c

OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

lisp: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o lisp

run: lisp
	./lisp

clean:
	rm -f $(OBJECTS)
