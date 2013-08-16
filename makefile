
CC=gcc

libfuncq:
ifeq ($(OS),Windows_NT)
	$(CC) -c -o $@.o function_queue.c
	ar rcs $@.a $@.o
else
	$(CC) -shared -fpic -o $@.so function_queue.c
endif

libqthread: libfuncq
ifeq ($(OS),Windows_NT)
	$(CC) -c -o $@.o threading_queue.c
	ar rcs $@.a $@.o
else
	$(CC) -shared -fpic -o $@.so threading_queue.c
endif

all : libqthread

: all

clean:
	rm -f libfuncq.a libfuncq.so libqthread.a libqthread.so
