
#include "threading_queue.h"

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

static void*
get_and_run(struct threading_queue* tq)
{
	struct function_queue_element fqe;

	do {
		if(fq_pop(tq->fq, &fqe, 1) == 0)
			fqe.func(fqe.arg);

		usleep(tq->delay);
	} while(1);
}

int
tq_init(struct threading_queue* tq, struct threading_queue_startup_info* tqsi)
{
	int ret = 0;
	tq->fq = tqsi->fq;
	tq->max_threads = tqsi->max_threads;
	tq->delay = tqsi->delay;
	tq->threads = malloc(tqsi->max_threads * sizeof(pthread_t));

	if(tq->threads == NULL)
		ret = ENOMEM;

	return ret;
}

int
tq_destroy(struct threading_queue* tq)
{
	free(tq->threads);
	return 0;
}

int
tq_start(struct threading_queue* tq)
{
	int ret = 0;
	unsigned int i = 0;

	for(i = 0; i < tq->max_threads; ++i)
		if(pthread_create(&tq->threads[i], 0,
				(void*(*)(void*)) get_and_run, tq) == 0)
			++ret;

	return ret;
}

int
tq_stop(struct threading_queue* tq)
{
	unsigned int i = 0;

	for(i = 0; i < tq->max_threads; ++i)
		pthread_cancel(tq->threads[i]);

	return 0;
}

