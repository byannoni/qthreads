
/*
 * Copyright 2015 Brandon Yannoni
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "threading_queue.h"

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>

#include "pt_error.h"

static void*
get_and_run(void* arg)
{
	struct threading_queue* tq = NULL;
	struct function_queue_element fqe;

	if(arg == NULL)
		return NULL;

	tq = arg;

	do {
		/* NOTE: no errors are defined for sched_yield() */
		(void) sched_yield();
		pthread_testcancel();

		if(fq_pop(tq->fq, &fqe, 1) == PT_SUCCESS)
			fqe.func(fqe.arg);

	} while(1);
}

enum pt_error
tq_init(struct threading_queue* tq, struct threading_queue_startup_info* tqsi)
{
	enum pt_error ret = 0;

	tq->fq = tqsi->fq;
	tq->max_threads = tqsi->max_threads;
	tq->threads = malloc(tqsi->max_threads * sizeof(pthread_t));

	if(tq->threads == NULL)
		ret = PT_EMALLOC;

	return ret;
}

enum pt_error
tq_destroy(struct threading_queue* tq)
{
	free(tq->threads);
	return PT_SUCCESS;
}

enum pt_error
tq_start(struct threading_queue* tq, int* started)
{
	enum pt_error ret = PT_SUCCESS;
	unsigned int i = 0;

	if(started != NULL) {
		*started = 0;

		for(i = 0; i < tq->max_threads; ++i)
			if(pthread_create(&tq->threads[i], 0, get_and_run, tq) != 0)
				ret = PT_EPTCANCEL;
			else
				++*started;
	} else {
		for(i = 0; i < tq->max_threads; ++i)
			if(pthread_create(&tq->threads[i], 0, get_and_run, tq) != 0)
				ret = PT_EPTCANCEL;
	}


	return ret;
}


enum pt_error
tq_stop(struct threading_queue* tq)
{
	enum pt_error ret = PT_SUCCESS;
	unsigned int i = 0;

	for(i = 0; i < tq->max_threads; ++i)
		pthread_cancel(tq->threads[i]);

	return ret;
}

