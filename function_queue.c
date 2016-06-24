
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

#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "function_queue.h"
#include "qterror.h"

static void release_mutex(void* m)
{
	(void) pthread_mutex_unlock((pthread_mutex_t*) m);
}

enum qterror
fqinit(struct function_queue* q, enum fqtype type, unsigned max_elements)
{
	enum qterror ret = QTSUCCESS;
	int pml = 0;

	assert(q != NULL);
	q->size = 0;
	q->max_elements = max_elements;
	q->type = type;

	switch(q->type) {
	case FQTYPE_IA:
		q->dispatchtable = &fqdispatchtableia;
		break;
	case FQTYPE_LAST:
		ret = QTEINVALID;
		goto end;
	}

	if((pml = pthread_mutex_init(&q->lock, NULL)) == 0) {
		pml = pthread_cond_init(&q->wait, NULL);

		if(pml) {
			/* ignore more errors at this point */
			(void) pthread_mutex_destroy(&q->lock);
			ret = QTEPTCINIT;
		} else {
			assert(q->dispatchtable->init != NULL);
			ret = q->dispatchtable->init(q, max_elements);

			if(ret != QTSUCCESS) {
				/* ignore more errors at this point */
				(void) pthread_mutex_destroy(&q->lock);
				(void) pthread_cond_destroy(&q->wait);
			}
		}
	} else {
		ret = QTEPTMINIT;
	}

end:
	return ret;
}

enum qterror
fqdestroy(struct function_queue* q)
{
	enum qterror ret = QTSUCCESS;
	int pmd = 0;

	assert(q != NULL);
	pmd = pthread_mutex_destroy(&q->lock);

	if(pmd == 0) {
		pmd = pthread_cond_destroy(&q->wait);

		if(pmd == 0) {
			assert(q->dispatchtable != NULL);
			assert(q->dispatchtable->destroy != NULL);
			ret = q->dispatchtable->destroy(q);
		} else {
			ret = QTEPTCDESTROY;
		}
	} else {
		ret = QTEPTMDESTROY;
	}

	return ret;
}

enum qterror
fqpush(struct function_queue* q, void (*func)(void*), void* arg, int block)
{
	enum qterror ret = QTSUCCESS;
	int pml = 0;

	assert(q != NULL);

	if(block != 0)
		pml = pthread_mutex_lock(&q->lock);
	else
		pml = pthread_mutex_trylock(&q->lock);

	if(pml == 0) {
		int isfull = 0;

		fqisfull(q, &isfull);

		if(isfull != 0) { /* overflow */
			ret = QTEFQFULL;
		} else {
			assert(q->dispatchtable != NULL);
			assert(q->dispatchtable->push != NULL);
			ret = q->dispatchtable->push(q, func, arg, block);

			if(ret == QTSUCCESS) {
				++q->size;

				/* Only signal if the queue was empty before */
				if(q->size == 1)
					(void) pthread_cond_signal(&q->wait);
			}
		}

		pml = pthread_mutex_unlock(&q->lock);

		if(pml != 0)
			ret = QTEPTMUNLOCK;
	} else {
		if(block == 0)
			ret = QTEPTMTRYLOCK;
		else
			ret = QTEPTMLOCK;
	}

	return ret;
}

enum qterror
fqpop(struct function_queue* q, struct function_queue_element* e, int block)
{
	enum qterror ret = QTSUCCESS;
	int pml = 0;

	assert(q != NULL);
	assert(e != NULL);

	if(block != 0)
		pml = pthread_mutex_lock(&q->lock);
	else
		pml = pthread_mutex_trylock(&q->lock);

	if(pml == 0) {
		int isempty = 0;

		fqisempty(q, &isempty);

		if(isempty) {
			if(block) {
				pthread_cleanup_push(release_mutex, &q->lock);

				do {
					pthread_cond_wait(&q->wait, &q->lock);
					fqisempty(q, &isempty);
				} while(isempty);

				pthread_cleanup_pop(0);
			} else {
				ret = QTEFQEMPTY;
			}
		}

		/*
		 * NOTE: This cannot be an else because isempty could change if
		 * we block
		 * */
		if(!isempty) {
			assert(q->dispatchtable != NULL);
			assert(q->dispatchtable->pop != NULL);
			ret = q->dispatchtable->pop(q, e, block);

			if(ret == QTSUCCESS)
				--q->size;
		}

		pml = pthread_mutex_unlock(&q->lock);

		if(pml != 0)
			ret = QTEPTMUNLOCK;
	} else {
		if(block == 0)
			ret = QTEPTMTRYLOCK;
		else
			ret = QTEPTMLOCK;
	}

	return ret;
}

enum qterror
fqpeek(struct function_queue* q, struct function_queue_element* e, int block)
{
	enum qterror ret = QTSUCCESS;
	int pml = 0;

	assert(q != NULL);
	assert(e != NULL);

	if(block != 0)
		pml = pthread_mutex_lock(&q->lock);
	else
		pml = pthread_mutex_trylock(&q->lock);

	if(pml == 0) {
		int isempty = 0;

		/* fqisempty() always succeeds */
		(void) fqisempty(q, &isempty);

		if(isempty) {
			if(block) {
				pthread_cleanup_push(release_mutex, &q->lock);

				do {
					pthread_cond_wait(&q->wait, &q->lock);
					fqisempty(q, &isempty);
				} while(isempty);

				pthread_cleanup_pop(0);
			} else {
				ret = QTEFQEMPTY;
			}
		}

		/*
		 * NOTE: This cannot be an else because isempty could change
		 * if we block.
		 */
		if(!isempty) {
			assert(q->dispatchtable != NULL);
			assert(q->dispatchtable->peek != NULL);
			ret = q->dispatchtable->peek(q, e, block);
		}

		pml = pthread_mutex_unlock(&q->lock);

		if(pml != 0)
			ret = QTEPTMUNLOCK;
	} else {
		if(block == 0)
			ret = QTEPTMTRYLOCK;
		else
			ret = QTEPTMLOCK;
	}

	return ret;
}

enum qterror
fqisempty(struct function_queue* q, int* isempty)
{
	enum qterror ret = QTSUCCESS;

	assert(q != NULL);
	assert(isempty != NULL);
	*isempty = q->size == 0;
	return ret;
}

enum qterror
fqisfull(struct function_queue* q, int* isempty)
{
	enum qterror ret = QTSUCCESS;

	assert(q != NULL);
	assert(isempty != NULL);
	*isempty = q->size == q->max_elements;
	return ret;
}

