# Made by Solomon for Div0 CSQ
# Taken and adapted from the following sites:
# 1. http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
# 2. https://github.com/google/honggfuzz/blob/master/Makefile

CC=gcc
CFLAGS=-I.
DEPS = lib.h
OBJ = main.o lib.o
FINAL_BIN = iicsg2019_csq_demo
SETUP_SCRIPT = setup_slcan0

CLEAN_TARGETS = $(OBJ) $(FINAL_BIN)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJ)
	$(CC) -o $(FINAL_BIN) $^ $(CFLAGS)

clean:
	$(RM) -r $(CLEAN_TARGETS)

PREFIX ?= /usr/local
BIN_PATH=$(PREFIX)/bin

install: all
	mkdir -p -m 755 $${DESTDIR}$(BIN_PATH)
	install -m 755 $(FINAL_BIN) $${DESTDIR}$(BIN_PATH)
	install -m 755 $(SETUP_SCRIPT) $${DESTDIR}$(BIN_PATH)
