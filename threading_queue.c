
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
		pthread_testcancel();

		if(fq_pop(tq->fq, &fqe, 1) == PT_SUCCESS)
			fqe.func(fqe.arg);

	} while(1);
}

enum pt_error
tq_init(struct threading_queue* tq, struct threading_queue_startup_info* tqsi)
{
	enum pt_error ret = PT_SUCCESS;
	
	assert(tq != NULL);
	assert(tqsi != NULL);

	tq->fq = tqsi->fq;
	tq->max_threads = tqsi->max_threads;
	tq->threads = malloc(tq->max_threads * sizeof(pthread_t));

	if(tq->threads != NULL) {
		tq->start_errors.current = 0;
		tq->start_errors.errors = calloc(tq->max_threads, sizeof(int));

		if(tq->start_errors.errors == NULL){
			free(tq->threads);
			ret = PT_EMALLOC;
		}
	} else {
		ret = PT_EMALLOC;
	}

	return ret;
}

enum pt_error
tq_destroy(struct threading_queue* tq)
{
	assert(tq != NULL);
	free(tq->start_errors.errors);
	free(tq->threads);
	return PT_SUCCESS;
}

enum pt_error
tq_start(struct threading_queue* tq, int* started)
{
	enum pt_error ret = PT_SUCCESS;
	unsigned int i = 0;

	assert(tq != NULL);

	if(started != NULL) {
		*started = 0;

		for(i = 0; i < tq->max_threads; ++i) {
			if(pthread_create(&tq->threads[i], 0, get_and_run, tq)
					!= 0) {
				if(tq->start_errors.errors != NULL)
					tq->start_errors.errors[i] = errno;

				ret = PT_EPTCREATE;
			} else {
				++*started;
			}
		}
	} else {
		for(i = 0; i < tq->max_threads; ++i) {
			if(pthread_create(&tq->threads[i], 0, get_and_run, tq)
					!= 0) {
				if(tq->start_errors.errors != NULL)
					tq->start_errors.errors[i] = errno;

				ret = PT_EPTCREATE;
			}
		}
	}

	return ret;
}


enum pt_error
tq_stop(struct threading_queue* tq, int join)
{
	enum pt_error ret = PT_SUCCESS;
	unsigned int i = 0;

	assert(tq != NULL);

	for(i = 0; i < tq->max_threads; ++i)
		/* NOTE it is not a problem if pthread_cancel() fails */
		(void) pthread_cancel(tq->threads[i]);

	if(join)
		for(i = 0; i < tq->max_threads; ++i)
			(void) pthread_join(tq->threads[i], NULL);

	return ret;
}

enum pt_error
tq_start_get_ne(struct threading_queue* tq, int* out)
{
	enum pt_error ret = PT_SUCCESS;

	assert(tq != NULL);
	assert(out != NULL);
	ret = tq_start_get_e(tq, tq->start_errors.current, out);
	++tq->start_errors.current;
	return ret;
}

enum pt_error
tq_start_get_e(struct threading_queue* tq, size_t n, int* out)
{
	enum pt_error ret = PT_SUCCESS;

	assert(tq != NULL);
	assert(out != NULL);

	if(n < tq->max_threads) {
		*out = tq->start_errors.errors[n];
	} else {
		errno = EINVAL;
		ret = PT_EERRNO;
	}

	return ret;
}

