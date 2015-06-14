
OBJS=threading_queue.o function_queue.o pt_error.o
CFLAGS=-fpic -c -DNDEBUG -D_XOPEN_SOURCE=500
DFLAGS=-UNDEBUG -g -ansi -O2 -Wpedantic -Wall -Wextra -Werror -Wformat=2 -Wimplicit -Wparentheses -Wunused -Wuninitialized -Wstrict-aliasing -Wstrict-overflow=5 -Wfloat-equal -Wdeclaration-after-statement -Wundef -Wshadow -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wsizeof-pointer-memaccess -Waggregate-return -Wstrict-prototypes -Woverlength-strings -Wredundant-decls -Wnested-externs -Wc++-compat -Wno-error=c++-compat -Wmissing-prototypes -Wno-error=missing-prototypes -Wdisabled-optimization -Wno-error=disabled-optimization 

ifeq ($(CC),gcc)
	DFLAGS+=-Wdouble-promotion -Wunsafe-loop-optimizations -Wc90-c99-compat -Wjump-misses-init -Wlogical-op -Wnormalized=nfc -Wunsuffixed-float-constants 
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

function_queue.o: function_queue.c function_queue.h pt_error.h
	$(CC) $(CFLAGS) -o $@ $<

threading_queue.o: threading_queue.c threading_queue.h function_queue.h pt_error.h
	$(CC) $(CFLAGS) -o $@ $<

pt_error.o: pt_error.c pt_error.h
	$(CC) $(CFLAGS) -o $@ $<

libqthread: $(OBJS)
	ar rcs $@.a $^

debug : CFLAGS+= $(DFLAGS)
debug : all

: all

clean:
	$(RM) libqthread.a $(OBJS)

