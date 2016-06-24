
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

#include "qtpool.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include "qterror.h"

static void*
get_and_run(void* arg)
{
	struct function_queue_element fqe;
	struct qtpool* tq = NULL;

	if(arg == NULL)
		return NULL;

	tq = arg;

	do {
		pthread_testcancel();

		if(fqpop(tq->fq, &fqe, 1) == QTSUCCESS)
			fqe.func(fqe.arg);

	} while(1);
}

enum qterror
qtinit(struct qtpool* tq, struct qtpool_startup_info* tqsi)
{
	enum qterror ret = QTSUCCESS;
	
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
			ret = QTEMALLOC;
		}
	} else {
		ret = QTEMALLOC;
	}

	return ret;
}

enum qterror
qtdestroy(struct qtpool* tq)
{
	assert(tq != NULL);
	free(tq->start_errors.errors);
	free(tq->threads);
	return QTSUCCESS;
}

enum qterror
qtstart(struct qtpool* tq, int* started)
{
	enum qterror ret = QTSUCCESS;
	unsigned int i = 0;

	assert(tq != NULL);

	if(started != NULL)
		*started = 0;

	for(i = 0; i < tq->max_threads; ++i) {
		if(pthread_create(&tq->threads[i], NULL, get_and_run,
					tq) != 0) {
			if(tq->start_errors.errors != NULL)
				tq->start_errors.errors[i] = errno;

			ret = QTEPTCREATE;
		} else {
			if(started != NULL)
				++*started;
		}
	}

	return ret;
}


enum qterror
qtstop(struct qtpool* tq, int join)
{
	enum qterror ret = QTSUCCESS;
	unsigned int i = 0;

	assert(tq != NULL);

	for(i = 0; i < tq->max_threads; ++i) {
		int pc = pthread_cancel(tq->threads[i]);

		if(join) {
			if(pc == 0)
				(void) pthread_join(tq->threads[i], NULL);
			else
				(void) pthread_detach(tq->threads[i]);
		}
	}

	return ret;
}

enum qterror
qtstart_get_e(struct qtpool* tq, size_t n, int* out)
{
	enum qterror ret = QTSUCCESS;

	assert(tq != NULL);
	assert(out != NULL);

	if(n < tq->max_threads) {
		*out = tq->start_errors.errors[n];
	} else {
		errno = EINVAL;
		ret = QTEERRNO;
	}

	return ret;
}

