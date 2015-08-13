
#include <stdio.h>
#include "../threading_queue.h"

static pthread_mutex_t printer = PTHREAD_MUTEX_INITIALIZER;

void my_puts(void* arg)
{
	pthread_mutex_lock(&printer);
	printf("%p\n", arg);
	pthread_mutex_unlock(&printer);
}

void my_puts2(void* arg)
{
	volatile int a = 15;
	int i = 1;
	for(; i < 2000000; ++i)
		a = a % i;
	pthread_mutex_lock(&printer);
	printf("%p\n", arg);
	pthread_mutex_unlock(&printer);
}

int main(int argc, char** argv)
{
	struct function_queue fq;
	struct threading_queue_startup_info tqsi;
	struct threading_queue tq;
	size_t threads = 4;
	int i = 0;
	int tqse[4];

	puts("fq_init");
	fq_init(&fq, FQTYPE_IA, 25);
	tqsi.fq = &fq;
	tqsi.max_threads = threads;
	puts("tq_init");
	tq_init(&tq, &tqsi);
	puts("start");
	tq_start(&tq, tqse);
	puts("pushing");

	for(i = 0; i < 100; ++i) {
		fq_push(&fq, i %3 ? my_puts2 : my_puts, (void*) i, 1);
		printf("pushed %p with %p\n", e.arg, e.func);
	}
	
	sleep(2);
	tq_stop(&tq, 1);
	puts("stopped");
	tq_destroy(&tq);
	fq_destroy(&fq);
	return 0;
}
