#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#include "function_queue.h"

static pthread_mutexattr_t attr;
static pthread_once_t init_attr_control = PTHREAD_ONCE_INIT;

static struct {
	int init;
	int settype;
} init_attr_ret;

static void
init_attr(void)
{
	init_attr_ret.init = pthread_mutexattr_init(&attr);

	if(init_attr_ret.init == 0)
		init_attr_ret.settype = pthread_mutexattr_settype(&attr,
				PTHREAD_MUTEX_RECURSIVE);
}

int
fq_init(struct function_queue* q, unsigned max_elements)
{
	int ret = pthread_once(&init_attr_control, init_attr);

	if(ret == 0) {
		if(init_attr_ret.init != 0 || init_attr_ret.settype != 0) {
			if(init_attr_ret.init != 0)
				ret = EMUTEXATTR_INIT;

			if(init_attr_ret.settype != 0)
				ret = EMUTEXATTR_SETTYPE;
		} else if((ret = pthread_mutex_init(&q->lock,&attr)) == 0) {
			q->front = 0;
			q->back = 0;
			q->size = 0;
			q->max_elements = max_elements;
			q->elements = malloc(q->max_elements *
					sizeof(struct function_queue_element));

			if(q->elements == NULL) {
				ret = errno;
				pthread_mutex_destroy(&q->lock);
			}
		}
	}

	return ret;
}

int
fq_destroy(struct function_queue* q)
{
	int ret = pthread_mutex_destroy(&q->lock);

	if(ret == 0) {
		free((struct function_queue_element*) q->elements);
		q->elements = NULL;
	}

	return ret;
}

int
fq_push(struct function_queue* q, struct function_queue_element e, int block)
{
	int ret;

	if(block != 0)
		ret = pthread_mutex_lock(&q->lock);
	else
		ret = pthread_mutex_trylock(&q->lock);

	if(ret == 0) {
		if(fq_is_full(q, block) != 0) { /* overflow */
			ret = ERANGE;
		} else {
			++q->size;

			if(++q->back == q->max_elements)
				q->back = 0;

			q->elements[q->back] = e;
		}

		pthread_mutex_unlock(&q->lock);
	}

	return ret;
}

int
fq_pop(struct function_queue* q, struct function_queue_element* e, int block)
{
	int ret;

	if(block != 0)
		ret = pthread_mutex_lock(&q->lock);
	else
		ret = pthread_mutex_trylock(&q->lock);

	if(ret == 0) {
		if(fq_is_empty(q, block) != 0) { /* underflow */
			ret = ERANGE;
		} else {
			--q->size;

			if(++q->front == q->max_elements)
				q->front = 0;

			*e = q->elements[q->front];
		}

		pthread_mutex_unlock(&q->lock);
	}

	return ret;
}

int
fq_peek(struct function_queue* q, struct function_queue_element* e, int block)
{
	int ret;

	if(block)
		ret = pthread_mutex_lock(&q->lock);
	else
		ret = pthread_mutex_trylock(&q->lock);

	if(ret == 0) {
		if(fq_is_empty(q, block)) {
			ret = ERANGE;
		} else {
			unsigned tmp = q->front + 1;

			if(tmp == q->max_elements)
				tmp = 0;

			*e = q->elements[tmp];
		}

		pthread_mutex_unlock(&q->lock);
	}

	return ret;
}

int
fq_is_empty(struct function_queue* q, int block)
{
	int ret;

	if(block != 0)
		ret = pthread_mutex_lock(&q->lock);
	else
		ret = pthread_mutex_trylock(&q->lock);

	if(ret == 0) {
		ret = q->size == 0;
		pthread_mutex_unlock(&q->lock);
	}

	return ret;
}

int
fq_is_full(struct function_queue* q, int block)
{
	int ret;

	if(block != 0)
		ret = pthread_mutex_lock(&q->lock);
	else
		ret = pthread_mutex_trylock(&q->lock);

	if(ret == 0) {
		ret = q->size == q->max_elements;
		pthread_mutex_unlock(&q->lock);
	}

	return ret;
}

