
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
#include "pt_error.h"

static void release_mutex(void* m)
{
	(void) pthread_mutex_unlock((pthread_mutex_t*) m);
}

enum pt_error
fq_init(struct function_queue* q, enum fqtype type, unsigned max_elements)
{
	enum pt_error ret = PT_SUCCESS;
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
		ret = PT_EINVALID;
		goto end;
	}

	if((pml = pthread_mutex_init(&q->lock, NULL)) == 0) {
		if(pml == 0) {
			pml = pthread_cond_init(&q->wait, NULL);

			if(pml) {
				/* ignore more errors at this point */
				(void) pthread_mutex_destroy(&q->lock);
				ret = PT_EPTCINIT;
			} else {
				assert(q->dispatchtable->init != NULL);
				ret = q->dispatchtable->init(q, max_elements);

				if(ret != PT_SUCCESS) {
					/* ignore more errors at this point */
					(void) pthread_mutex_destroy(&q->lock);
					(void) pthread_cond_destroy(&q->wait);
				}
			}
		} else {
			ret = PT_EPTMUNLOCK;
		}
	} else {
		ret = PT_EPTMLOCK;
	}

end:
	return ret;
}

enum pt_error
fq_destroy(struct function_queue* q)
{
	enum pt_error ret = PT_SUCCESS;
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
			ret = PT_EPTCDESTROY;
		}
	} else {
		ret = PT_EPTMDESTROY;
	}

	return ret;
}

enum pt_error
fq_push(struct function_queue* q, void (*func)(void*), void* arg, int block)
{
	enum pt_error ret = PT_SUCCESS;
	int pml = 0;

	assert(q != NULL);

	if(block != 0)
		pml = pthread_mutex_lock(&q->lock);
	else
		pml = pthread_mutex_trylock(&q->lock);

	if(pml == 0) {
		int is_full = 0;

		fq_is_full(q, &is_full);

		if(is_full != 0) { /* overflow */
			ret = PT_EFQFULL;
		} else {
			assert(q->dispatchtable != NULL);
			assert(q->dispatchtable->push != NULL);
			ret = q->dispatchtable->push(q, func, arg, block);

			if(ret == PT_SUCCESS) {
				++q->size;

				/* Only signal if the queue was empty before */
				if(q->size == 1)
					(void) pthread_cond_signal(&q->wait);
			}
		}

		pml = pthread_mutex_unlock(&q->lock);

		if(pml != 0)
			ret = PT_EPTMUNLOCK;
	} else {
		if(block == 0)
			ret = PT_EPTMTRYLOCK;
		else
			ret = PT_EPTMLOCK;
	}

	return ret;
}

enum pt_error
fq_pop(struct function_queue* q, struct function_queue_element* e, int block)
{
	enum pt_error ret = PT_SUCCESS;
	int pml = 0;

	assert(q != NULL);
	assert(e != NULL);

	if(block != 0)
		pml = pthread_mutex_lock(&q->lock);
	else
		pml = pthread_mutex_trylock(&q->lock);

	if(pml == 0) {
		int is_empty = 0;

		fq_is_empty(q, &is_empty);

		if(is_empty) {
			if(block) {
				pthread_cleanup_push(release_mutex, &q->lock);

				do {
					pthread_cond_wait(&q->wait, &q->lock);
					fq_is_empty(q, &is_empty);
				} while(is_empty);

				pthread_cleanup_pop(0);
			} else {
				ret = PT_EFQEMPTY;
			}
		}

		/*
		 * NOTE: This cannot be an else because is_empty could change if
		 * we block
		 * */
		if(!is_empty) {
			assert(q->dispatchtable != NULL);
			assert(q->dispatchtable->pop != NULL);
			ret = q->dispatchtable->pop(q, e, block);

			if(ret == PT_SUCCESS)
				--q->size;
		}

		pml = pthread_mutex_unlock(&q->lock);

		if(pml != 0)
			ret = PT_EPTMUNLOCK;
	} else {
		if(block == 0)
			ret = PT_EPTMTRYLOCK;
		else
			ret = PT_EPTMLOCK;
	}

	return ret;
}

enum pt_error
fq_peek(struct function_queue* q, struct function_queue_element* e, int block)
{
	enum pt_error ret = PT_SUCCESS;
	int pml = 0;

	assert(q != NULL);
	assert(e != NULL);

	if(block != 0)
		pml = pthread_mutex_lock(&q->lock);
	else
		pml = pthread_mutex_trylock(&q->lock);

	if(pml == 0) {
		int is_empty = 0;

		/* fq_is_empty() always succeeds */
		(void) fq_is_empty(q, &is_empty);

		if(is_empty) {
			if(block) {
				pthread_cleanup_push(release_mutex, &q->lock);

				do {
					pthread_cond_wait(&q->wait, &q->lock);
					fq_is_empty(q, &is_empty);
				} while(is_empty);

				pthread_cleanup_pop(0);
			} else {
				ret = PT_EFQEMPTY;
			}
		}

		/*
		 * NOTE: This cannot be an else because is_empty could change
		 * if we block.
		 */
		if(!is_empty) {
			assert(q->dispatchtable != NULL);
			assert(q->dispatchtable->peek != NULL);
			ret = q->dispatchtable->peek(q, e, block);
		}

		pml = pthread_mutex_unlock(&q->lock);

		if(pml != 0)
			ret = PT_EPTMUNLOCK;
	} else {
		if(block == 0)
			ret = PT_EPTMTRYLOCK;
		else
			ret = PT_EPTMLOCK;
	}

	return ret;
}

enum pt_error
fq_is_empty(struct function_queue* q, int* is_empty)
{
	enum pt_error ret = PT_SUCCESS;

	assert(q != NULL);
	assert(is_empty != NULL);
	*is_empty = q->size == 0;
	return ret;
}

enum pt_error
fq_is_full(struct function_queue* q, int* is_empty)
{
	enum pt_error ret = PT_SUCCESS;

	assert(q != NULL);
	assert(is_empty != NULL);
	*is_empty = q->size == q->max_elements;
	return ret;
}

