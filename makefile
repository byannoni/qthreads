
COBJS=threading_queue_c.o function_queue_c.o
CPPOBJS=function_queue_cpp.o threading_queue_cpp.o
OBJS=$(COBJS) $(CPPOBJS)
CPPFLAGS=-fpic -c -DNDEBUG
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

all : libqthread libqthread_cpp

function_queue_c.o: function_queue.c function_queue.h
	$(CC) $(CFLAGS) -o $@ $<

threading_queue_c.o: threading_queue.c threading_queue.h function_queue.h
	$(CC) $(CFLAGS) -o $@ $<

libqthread: $(OBJS)
	ar rcs $@.a $^

function_queue_cpp.o: function_queue.cpp function_queue.h
	$(CXX) $(CPPFLAGS) -o $@ $<

threading_queue_cpp.o: threading_queue.cpp threading_queue.h function_queue.h
	$(CXX) $(CPPFLAGS) -o $@ $<

libqthread_cpp: libqthread $(CPPOBJS)
	ar rs $<.a $(CPPOBJS)

debug : CFLAGS+= $(DFLAGS)
debug : all

: all

clean:
	$(RM) libqthread.a $(OBJS)

