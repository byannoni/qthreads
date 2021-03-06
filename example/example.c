
#include <stdio.h>
#include "../qtpool.h"

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
	struct qtpool_startup_info tqsi;
	struct qtpool tq;
	size_t threads = 4;
	int i = 0;
	int tqse[4];

	puts("fq_init");
	fq_init(&fq, FQTYPE_IA, 25);
	tqsi.fq = &fq;
	tqsi.max_threads = threads;
	puts("qtinit");
	qtinit(&tq, &tqsi);
	puts("start");
	qtstart(&tq, tqse);
	puts("pushing");

	for(i = 0; i < 100; ++i) {
		fq_push(&fq, i %3 ? my_puts2 : my_puts, (void*) i, 1);
		printf("pushed %p with %p\n", i %3 ? my_puts2 : my_puts, (void*) i);
	}
	
	sleep(2);
	qtstop(&tq, 1);
	puts("stopped");
	qtdestroy(&tq);
	fq_destroy(&fq);
	return 0;
}
