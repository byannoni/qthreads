
OBJS=threading_queue.o function_queue.o
CFLAGS=-fpic -c -DNDEBUG
DFLAGS=-UNDEBUG -D_DEBUG -pedantic -g -Werror

ifeq ($(CC),gcc)
	DFLAGS+=-Wall -Wextra
else
	ifeq ($(CC),clang)
		DFLAGS+=-Weverything -Wno-padded
	endif
endif

ifeq ($(OS),Windows_NT)
	RM=del
else
	RM=rm -f
endif

all : libqthread

function_queue.o: function_queue.c function_queue.h
	$(CC) $(CFLAGS) -o $@ $<

threading_queue.o: threading_queue.c threading_queue.h function_queue.h
	$(CC) $(CFLAGS) -o $@ $<

libqthread: $(OBJS)
	ar rcs $@.a $^

debug : CFLAGS+= $(DFLAGS)
debug : all

: all

clean:
	$(RM) libqthread.a $(OBJS)

