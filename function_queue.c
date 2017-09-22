
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

#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "function_queue_element.h"
#include "function_queue.h"
#include "qterror.h"

#include "fq/indexed_array_queue.h"
#include "fq/linked_list_queue.h"

/*
 * This procedure is a wrapper around the mutex unlock procedure so that
 * the mutex can be unlocked in a cleanup handler. The variable m is a
 * pointer to the mutex to unlock. The value of m must not be NULL.
 */
static void
release_mutex(void* m)
{
	(void) pthread_mutex_unlock((pthread_mutex_t*) m);
}

/*
 * This procedure initializes a function queue based on the given type.
 * The variable max_elements is the maximum number of elements which can
 * be stored in the queue. The variable type indicated which dispatch
 * table to use for internal queue procedures. The procedure returns an
 * error code to indicate its status. The value of q must not be NULL.
 */
enum qterror
fqinit(struct function_queue* q, enum fqtype type, unsigned max_elements)
{
	enum qterror ret = QTSUCCESS;

	assert(q != NULL);
	q->size = 0;
	q->max_elements = max_elements;
	q->type = type;

	switch(q->type) {
	case FQTYPE_IA:
		q->dispatchtable = &fqdispatchtableia;
		break;
	case FQTYPE_LL:
		q->dispatchtable = &fqdispatchtablell;
		break;
	case FQTYPE_LAST:
		return QTEINVALID;
	}

	if(pthread_mutex_init(&q->lock, NULL) != 0)
		return QTEPTMINIT;

	if(pthread_cond_init(&q->wait, NULL) != 0) {
		/* ignore more errors at this point */
		(void) pthread_mutex_destroy(&q->lock);
		return QTEPTCINIT;
	}

	assert(q->dispatchtable->init != NULL);
	ret = q->dispatchtable->init(q, max_elements);

	if(ret != QTSUCCESS) {
		/* ignore more errors at this point */
		(void) pthread_mutex_destroy(&q->lock);
		(void) pthread_cond_destroy(&q->wait);
	}

	return ret;
}

/*
 * This procedure destroys the given queue. An attempt to use the object
 * after destroying it results in undefined behavior. The object can be
 * reinitialized by fqinit(). This procedure always succeeds. The value
 * of q must not be NULL.
 */
enum qterror
fqdestroy(struct function_queue* q)
{
	assert(q != NULL);

	if(pthread_mutex_destroy(&q->lock) != 0)
		return QTEPTMDESTROY;

	if(pthread_cond_destroy(&q->wait) != 0)
		return QTEPTCDESTROY;

	assert(q->dispatchtable != NULL);
	assert(q->dispatchtable->destroy != NULL);
	return q->dispatchtable->destroy(q);
}

/*
 * This procedure pushes the given function pointer onto the queue. The 
 * function pointer is stored with the given argument arg so the value
 * can be passed to it. This procedure may block if the value of block
 * is non-zero. The procedure returns an error code to indicate its
 * status. The value of q must not be NULL.
 */
enum qterror
fqpush(struct function_queue* q, void (*func)(void*), void* arg, int block)
{
	enum qterror ret = QTSUCCESS;
	int isfull = 0;

	assert(q != NULL);

	if(block) {
		if(pthread_mutex_lock(&q->lock) != 0)
			return QTEPTMLOCK;
	} else {
		if(pthread_mutex_trylock(&q->lock) != 0)
			return QTEPTMTRYLOCK;
	}

	(void) fqisfull(q, &isfull);

	if(isfull != 0) { /* overflow */
		ret = QTEFQFULL;
		goto unlock_queue_mutex;
	}

	assert(q->dispatchtable != NULL);
	assert(q->dispatchtable->push != NULL);
	ret = q->dispatchtable->push(q, func, arg, block);

	if(ret == QTSUCCESS) {
		++q->size;

		/* Only signal if the queue was empty before */
		if(q->size == 1)
			(void) pthread_cond_signal(&q->wait);
	}

unlock_queue_mutex:
	if(pthread_mutex_unlock(&q->lock) != 0)
		if(ret != QTSUCCESS)
			ret = QTEPTMUNLOCK;

	return ret;
}

/*
 * This procedure pops a function pointer from the queue. The function
 * pointer and its information is stored in a function queue element.
 * The value of this function queue element is copied to the address
 * pointed to by the variable e and then removed from the queue. This
 * procedure may block if the value of block is non-zero. The procedure
 * returns an error code to indicate its status. The value of q must not
 * be NULL. The value of e must not be NULL.
 */
enum qterror
fqpop(struct function_queue* q, struct function_queue_element* e, int block)
{
	enum qterror ret = QTSUCCESS;
	int isempty = 0;

	assert(q != NULL);
	assert(e != NULL);

	if(block) {
		if(pthread_mutex_lock(&q->lock) != 0)
			return QTEPTMLOCK;
	} else {
		if(pthread_mutex_trylock(&q->lock) != 0)
			return QTEPTMTRYLOCK;
	}

	fqisempty(q, &isempty);

	if(isempty) {
		if(!block) {
			ret = QTEFQEMPTY;
			goto unlock_queue_mutex;
		}

		pthread_cleanup_push(release_mutex, &q->lock);

		do {
			pthread_cond_wait(&q->wait, &q->lock);
			fqisempty(q, &isempty);
		} while(isempty);

		pthread_cleanup_pop(0);
	}

	/*
	 * NOTE: This cannot be an else because isempty could change if
	 * we block.
	 */
	if(!isempty) {
		assert(q->dispatchtable != NULL);
		assert(q->dispatchtable->pop != NULL);
		ret = q->dispatchtable->pop(q, e, block);

		if(ret == QTSUCCESS)
			--q->size;
	}

unlock_queue_mutex:
	if(pthread_mutex_unlock(&q->lock) != 0)
		if(ret != QTSUCCESS)
			ret = QTEPTMUNLOCK;

	return ret;
}

/*
 * This procedure peeks at a function pointer from the queue. The
 * function pointer and its information is stored in a function queue
 * element. The value of this function queue element is copied to the
 * address pointed to by the variable e. This procedure may block if the
 * value of block is non-zero. The procedure returns an error code to
 * indicate its status. The value of q must not be NULL. The value of e
 * must not be NULL.
 */
enum qterror
fqpeek(struct function_queue* q, struct function_queue_element* e, int block)
{
	enum qterror ret = QTSUCCESS;
	int isempty = 0;

	assert(q != NULL);
	assert(e != NULL);

	if(block != 0) {
		if(pthread_mutex_lock(&q->lock) != 0)
			return QTEPTMLOCK;
	} else {
		if(pthread_mutex_trylock(&q->lock) != 0)
			return QTEPTMTRYLOCK;
	}

	/* fqisempty() always succeeds */
	(void) fqisempty(q, &isempty);

	if(isempty) {
		if(!block) {
			ret = QTEFQEMPTY;
			goto unlock_queue_mutex;
		}

		pthread_cleanup_push(release_mutex, &q->lock);

		do {
			pthread_cond_wait(&q->wait, &q->lock);
			fqisempty(q, &isempty);
		} while(isempty);

		pthread_cleanup_pop(0);
	}

	/*
	 * NOTE: This cannot be an else because isempty could change if
	 * we block.
	 */
	if(!isempty) {
		assert(q->dispatchtable != NULL);
		assert(q->dispatchtable->peek != NULL);
		ret = q->dispatchtable->peek(q, e, block);
	}

unlock_queue_mutex:
	if(pthread_mutex_unlock(&q->lock) != 0)
		if(ret == QTSUCCESS)
			return QTEPTMUNLOCK;

	return ret;
}

/*
 * This procedure checks if the given queue is empty. It sets the value
 * at the address pointed to by isempty to 0 if the queue is empty.
 * Otherwise, it sets the value pointed to by isempty to non-zero. This
 * procedure always succeeds. The value of q must not be NULL. The value
 * of isempty must not be NULL.
 */
enum qterror
fqisempty(struct function_queue* q, int* isempty)
{
	assert(q != NULL);
	assert(isempty != NULL);
	*isempty = q->size == 0;
	return QTSUCCESS;
}

/*
 * This procedure checks if the given queue is full. It sets the value
 * at the address pointed to by isfull to 0 if the queue is full.
 * Otherwise, it sets the value pointed to by isfull to non-zero. This
 * procedure always succeeds. The value of q must not be NULL. The value
 * of isfull must not be NULL.
 */
enum qterror
fqisfull(struct function_queue* q, int* isfull)
{
	assert(q != NULL);
	assert(isfull != NULL);
	*isfull = q->size == q->max_elements;
	return QTSUCCESS;
}

