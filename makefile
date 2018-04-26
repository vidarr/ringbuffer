CC=gcc
LN=gcc

CFLAGS=-Wall --std=c11 -g

.phony: all
all: build/ringbuffer_test

build/%.o: src/%.c build include/ringbuffer.h
	$(CC) $(CFLAGS) -c $< -o $@

build/%_test.o: test/%_test.c build include/ringbuffer.h
	$(CC) $(CFLAGS) -c $< -o $@

build/ringbuffer_test: build/ringbuffer_test.o
	$(LN) $^ -o $@

build:
	mkdir -p build

clean:
	rm -rf build
