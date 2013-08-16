
CC=gcc

all : libqthread

libfuncq:
ifeq ($(OS),Windows_NT)
	$(CC) -c -o $@.o function_queue.c
	ar rcs $@.a $@.o
else
	$(CC) -shared -fpic -o $@.so function_queue.c
endif

libqthread: threading_queue.c libfuncq
ifeq ($(OS),Windows_NT)
	$(CC) -c -o $@.o threading_queue.c
	ar rcs $@.a $@.o
else
	$(CC) -shared -fpic -o $@.so threading_queue.c
endif

: all

clean:
	rm -f libfuncq.a libfuncq.so libqthread.a libqthread.so
