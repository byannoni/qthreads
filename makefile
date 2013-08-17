
CC=gcc
OBJS=threading_queue.o function_queue.o

ifeq ($(OS),Windows_NT)
	RM=del
else
	RM=rm -f
endif

all : libqthread

function_queue.o: function_queue.c function_queue.h
	$(CC) -Wall -Wextra -fpic -c -o $@ $<

threading_queue.o: threading_queue.c threading_queue.h function_queue.h
	$(CC) -Wall -Wextra -fpic -c -Wl,--no-as-needed -o $@ $<

libqthread: $(OBJS)
	ar rcs $@.a $^

: all

clean:
	$(RM) libqthread.a $(OBJS)

