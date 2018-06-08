CC=gcc
LN=gcc

CFLAGS=-Wall --std=c11 -g

.phony: all
all: build/ringbuffer_test build/cached_ringbuffer_test build/buffercache_test build/caching_ringbuffer_test

build/%.o: src/%.c build include/ringbuffer.h
	$(CC) $(CFLAGS) -c $< -o $@

build/%_test.o: test/%_test.c build include/ringbuffer.h test/test_helper.h build/test_helper.o
	$(CC) $(CFLAGS) -c $< -o $@

build/test_helper.o: test/test_helper.c test/test_helper.h
	$(CC) $(CFLAGS) -c $< -o $@

build/%_test: build/%_test.o build/test_helper.o
	$(LN) $^ -o $@

build/caching_ringbuffer_test: build/caching_ringbuffer_test.o build/test_helper.o build/ringbuffer.o
	$(LN) $^ -o $@

build:
	mkdir -p build

clean:
	rm -rf build
