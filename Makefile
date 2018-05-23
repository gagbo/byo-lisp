CC = gcc

CFLAGS = -Wall -std=gnu11 -g
LDFLAGS = -ledit -lm

BUILD_DIR = build
SRC_DIR = src
SOURCES = parsing.c \
          mpc.c \
          evaluation.c \
          lval.c \
          lenv.c

OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

lisp: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o lisp

build/parsing.o: $(SRC_DIR)/evaluation.h $(SRC_DIR)/lenv.h $(SRC_DIR)/lval.h $(SRC_DIR)/mpc.h

build/evaluation.o: $(SRC_DIR)/lenv.h $(SRC_DIR)/mpc.h

build/lval.o: $(SRC_DIR)/lenv.h

build/lenv.o: $(SRC_DIR)/evaluation.h

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJECTS)
