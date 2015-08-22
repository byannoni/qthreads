
OBJS=qtpool.o function_queue.o pt_error.o indexed_array_queue.o
TESTEXECS=pt_error_test
CFLAGS=-fpic -DNDEBUG -D_XOPEN_SOURCE=500 -ansi -O2 -Wpedantic -Wall -Wextra -Werror -Wformat=2 -Wimplicit -Wparentheses -Wunused -Wuninitialized -Wstrict-aliasing -Wstrict-overflow=5 -Wfloat-equal -Wdeclaration-after-statement -Wundef -Wshadow -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wsizeof-pointer-memaccess -Waggregate-return -Wstrict-prototypes -Woverlength-strings -Wredundant-decls -Wnested-externs -Wc++-compat -Wno-error=c++-compat -Wmissing-prototypes -Wno-error=missing-prototypes -Wdisabled-optimization -Wno-error=disabled-optimization 
DFLAGS=-UNDEBUG -ggdb -O0

ifeq ($(CC),gcc)
	CFLAGS+=-Wdouble-promotion -Wunsafe-loop-optimizations -Wc90-c99-compat -Wjump-misses-init -Wlogical-op -Wnormalized=nfc -Wunsuffixed-float-constants 
else
	ifeq ($(CC),clang)
		CFLAGS+=-Weverything -Wno-padded
	endif
endif

ifeq ($(DEBUG),1)
	CFLAGS+= $(DFLAGS)
endif

ifeq ($(OS),Windows_NT)
	RM=del
else
	RM=rm -f
endif

all : libqthread

indexed_array_queue.o: fq/indexed_array_queue.c fq/indexed_array_queue.h pt_error.o
	$(CC) $(CFLAGS) -c -o $@ $<

function_queue.o: function_queue.c function_queue.h pt_error.o indexed_array_queue.o
	$(CC) $(CFLAGS) -c -o $@ $<

qtpool.o: qtpool.c qtpool.h function_queue.o pt_error.o
	$(CC) $(CFLAGS) -c -o $@ $<

pt_error.o: pt_error.c pt_error.h
	$(CC) $(CFLAGS) -c -o $@ $<

libqthread: $(OBJS)
	ar rcs $@.a $^

test: test/pt_error.c libqthread pt_error.c pt_error.c test/tinytest/tinytest.h
	$(CC) -pthread -o pt_error_test $<
	$(foreach TEST,$(TESTEXECS),./$(TEST))

: all

clean:
	$(RM) libqthread.a $(OBJS) $(TESTEXECS)

