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
init_attr( void )
{
	init_attr_ret.init = pthread_mutexattr_init( &attr );

	if( !init_attr_ret.init ) {
		init_attr_ret.settype = pthread_mutexattr_settype( &attr,
				PTHREAD_MUTEX_RECURSIVE );
	}
}

int
fq_init( struct function_queue* q, unsigned max_elements )
{
	int ret = pthread_once( &init_attr_control, init_attr );

	/*
	 * XXX
	 * According to the C11 standard, this could conflict with
	 * implementation-specific errno values. However, this case is
	 * very unlikely since all common errno values are positive. A
	 * separate function for checking init_attr_ret may be
	 * implemented in the future so these conditions will no longer
	 * bitwise-or against ret. 
	 *
	 * TODO
	 * Add constants for the bitwise-or's.
	 */
	if( !ret ) {
		if( init_attr_ret.init || init_attr_ret.settype ) {
			if( init_attr_ret.init ) {
				ret = ~-2; 
			}

			if( init_attr_ret.settype ) {
				ret |= ~-3;
			}
		} else if( !( ret || ( ret = pthread_mutex_init( &q->lock, &attr )))) {
			q->front = 0;
			q->back = 0;
			q->size = 0;
			q->max_elements = max_elements;
			q->elements = malloc( q->max_elements *
					sizeof( struct function_queue_element ));

			if( !q->elements ) {
				ret = errno;
				pthread_mutex_destroy( &q->lock );
			}
		}
	}

	return ret;
}

int
fq_destroy( struct function_queue* q )
{
	int ret = pthread_mutex_destroy( &q->lock );

	if( !ret ) {
		free((struct function_queue_element*) q->elements );
	}

	return ret;
}

int
fq_push( struct function_queue* q, struct function_queue_element e, int block )
{
	int ret;

	if( block ) {
		ret = pthread_mutex_lock( &q->lock );
	} else {
		ret = pthread_mutex_trylock( &q->lock );
	}

	if( !ret ) {
		if( fq_is_full( q, 1 )) { /* overflow */
			ret = ERANGE;
		} else {
			++q->size;

			if( ++q->back == q->max_elements ) {
				q->back = 0;
			}

			q->elements[q->back] = e;
		}

		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

int
fq_pop( struct function_queue* q, struct function_queue_element* e, int block )
{
	int ret;

	if( block ) {
		ret = pthread_mutex_lock( &q->lock );
	} else {
		ret = pthread_mutex_trylock( &q->lock );
	}

	if( !ret ) {
		if( fq_is_empty( q, 1 )) { /* underflow */
			ret = ERANGE;
		} else {
			--q->size;

			if( ++q->front == q->max_elements ) {
				q->front = 0;
			}

			*e = q->elements[q->front];
		}

		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

int
fq_peek( struct function_queue* q, struct function_queue_element* e, int block )
{
	int ret;

	if( block ) {
		ret = pthread_mutex_lock( &q->lock );
	} else {
		ret = pthread_mutex_trylock( &q->lock );
	}

	if( !ret ) {
		if( fq_is_empty( q, 1 )) {
			ret = ERANGE;
		} else {
			unsigned tmp = q->front + 1;

			if( tmp == q->max_elements ) {
				tmp = 0; 
			}

			*e = q->elements[tmp];
		}

		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

int
fq_is_empty( struct function_queue* q, int block )
{
	int ret;

	if( block ) {
		ret = pthread_mutex_lock( &q->lock );
	} else {
		ret = pthread_mutex_trylock( &q->lock );
	}

	if( !ret ) {
		ret = q->size == 0;
		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

int
fq_is_full( struct function_queue* q, int block )
{
	int ret;

	if( block ) {
		ret = pthread_mutex_lock( &q->lock );
	} else {
		ret = pthread_mutex_trylock( &q->lock );
	}

	if( !ret ) {
		ret = q->size == q->max_elements;
		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

