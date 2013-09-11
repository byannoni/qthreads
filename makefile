
CC=gcc
CPPC=g++
COBJS=threading_queue_c.o function_queue_c.o
CPPOBJS=function_queue_cpp.o threading_queue_cpp.o
OBJS=$(COBJS) $(CPPOBJS)

ifeq ($(OS),Windows_NT)
	RM=del
else
	RM=rm -f
endif

all : libqthread libqthread_cpp

function_queue_c.o: function_queue.c function_queue.h
	$(CC) -Wall -Wextra -fpic -c -o $@ $<

threading_queue_c.o: threading_queue.c threading_queue.h function_queue.h
	$(CC) -Wall -Wextra -fpic -c -o $@ $<

libqthread: $(COBJS)
	ar rcs $@.a $^

function_queue_cpp.o: function_queue.cpp function_queue.h
	$(CC) -Wall -Wextra -fpic -c -o $@ $<

threading_queue_cpp.o: threading_queue.cpp threading_queue.h function_queue.h
	$(CC) -Wall -Wextra -fpic -c -o $@ $<

libqthread_cpp: libqthread $(CPPOBJS)
	ar rs $<.a $(CPPOBJS)

: all

clean:
	$(RM) libqthread.a $(OBJS)

