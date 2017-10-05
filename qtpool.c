
/*
 * Copyright 2017 Brandon Yannoni
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

/*
 * This procedure repeatedly retrives a function from the function queue
 * and executes it. This runs until the calling thread is cancelled. The
 * argument is a pointer to an initialized qtpool object. This procedure
 * does not return unless the value of arg NULL.
 */
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

/*
 * This procedure initializes the qtpool object tq using the startup
 * information from tqsi. The procedure returns a qterror code to
 * indicate its status. The value of tq must not be NULL. The value of
 * tqsi must not be NULL.
 */
enum qterror
qtinit(struct qtpool* tq, struct qtpool_startup_info* tqsi)
{
	assert(tq != NULL);
	assert(tqsi != NULL);

	tq->fq = tqsi->fq;
	tq->max_threads = tqsi->max_threads;
	tq->threads = malloc(tq->max_threads * sizeof(pthread_t));

	if(tq->threads == NULL)
		return QTEMALLOC;

	tq->start_errors.errors = calloc(tq->max_threads, sizeof(int));

	if(tq->start_errors.errors == NULL){
		free(tq->threads);
		return QTEMALLOC;
	}

	return QTSUCCESS;
}

/*
 * This procedure destroys the given qtpool object. An attempt to use
 * the object after destroying it results in undefined behavior. The
 * object can be reinitialized by qtinit(). This procedure always
 * succeeds. The value of tq must not be NULL.
 */
enum qterror
qtdestroy(struct qtpool* tq)
{
	assert(tq != NULL);
	free(tq->start_errors.errors);
	free(tq->threads);
	return QTSUCCESS;
}

/*
 * This procedure starts the threads for the given pool. The number of
 * threads which start successfully is stored in the integer pointed to
 * by started if the value of started is not NULL. This procedure
 * returns an error code indicating its status. The value of tq must not
 * be NULL.
 */
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

/*
 * This procedure stops the threads in a given pool. The threads are
 * stopped by canceling them. If the value of join is not zero, the
 * threads are joined and the procedure blocks until all the threads
 * have terminated. Otherwise, the threads are detached and the
 * procedure does not block. This procedure always succeeds. The value
 * of tq must not be NULL.
 */
enum qterror
qtstop(struct qtpool* tq, int join)
{
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

	return QTSUCCESS;
}

/*
 * This procedure retrieves an errno value corresponding to the status
 * of the status of the creation of the thread of the given index n.
 * The value is then stored at the address pointed to by out. This
 * procedure returns an error code indicating its status. The value of
 * tq must not be NULL. The value of out must not be NULL.
 */
enum qterror
qtstart_get_e(struct qtpool* tq, size_t n, int* out)
{
	assert(tq != NULL);
	assert(out != NULL);

	if(n >= tq->max_threads)
		return QTEINVALID;

	*out = tq->start_errors.errors[n];
	return QTSUCCESS;
}

