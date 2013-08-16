
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#include "function_queue.h"

static pthread_mutexattr_t attr;
static pthread_once_t init_attr_control = PTHREAD_ONCE_INIT;

static volatile struct {
	int init;
	int settype;
} init_attr_ret;

static void init_attr( void )
{
	init_attr_ret.init = pthread_mutexattr_init( &attr );
	init_attr_ret.settype = pthread_mutexattr_settype( &attr,
			PTHREAD_MUTEX_RECURSIVE );
}


int function_queue_init( struct function_queue* q, unsigned max_elements )
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
	 */
	if( init_attr_ret.init ) {
		ret |= ~-2; 

		if( init_attr_ret.settype )
			ret |= ~-3; 
	} else if( !( ret || ( ret = pthread_mutex_init( &q->lock, &attr )))) {
		q->front = 0;
		q->back = 0;
		q->max_elements = max_elements;
		q->elements = malloc( max_elements *
				sizeof( struct function_queue_element ));

		if( !q->elements ) {
			ret = errno;
			pthread_mutex_destroy( &q->lock );
		}
	}

	return ret;
}

int function_queue_destroy( struct function_queue* q )
{
	int ret = pthread_mutex_destroy( &q->lock );

	if( !ret )
		free( q->elements );

	return ret;
}

int push( struct function_queue* q, struct function_queue_element e )
{
	int ret = pthread_mutex_trylock( &q->lock );

	if( !ret ) {
		unsigned tmp = q->back++;

		if( q->back == q->max_elements )
			q->back = 0;
		
		if( q->back == q->front ) { /* overflow */
			q->back = tmp;
			ret = ERANGE;
		} else {
			q->elements[q->back] = e;
		}

		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

int pop( struct function_queue* q, struct function_queue_element* e )
{
	int ret = pthread_mutex_trylock( &q->lock );

	if( !ret ) {
		unsigned tmp = q->front++;

		if( q->front == q->max_elements )
			q->front = 0;

		if( q->front == q->back ) { /* underflow */
			q->front = tmp;
			ret = ERANGE;
		} else {
			*e = q->elements[q->front];
		}

		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

int peek( struct function_queue* q, struct function_queue_element* e )
{
	int ret = pthread_mutex_trylock( &q->lock );

	if( !ret ) {
		unsigned tmp = q->front + 1;

		if( tmp == q->max_elements )
			tmp = 0;

		if( tmp == q->back ) /* underflow */
			ret = ERANGE;
		else
			*e = q->elements[tmp];

		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

int is_empty( struct function_queue* q )
{
	int ret = pthread_mutex_trylock( &q->lock );

	if( !ret ) {
		ret = q->front == q->back;
		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

int is_full( struct function_queue* q )
{
	int ret = pthread_mutex_trylock( &q->lock );

	if( !ret ) {
		unsigned tmp = q->back + 1;

		if( tmp == q->front || tmp == q->max_elements && q->front == 0 )
			ret = 1;

		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

int resize( struct function_queue* q, unsigned max_elements )
{
	int ret = pthread_mutex_trylock( &q->lock );

	if( !ret ) {
		struct function_queue_element* tmp = q->elements;
		q->elements = realloc( q->elements, max_elements *
				sizeof( struct function_queue_element ));
		ret = q->elements != tmp;
		pthread_mutex_unlock( &q->lock );
	}

	return ret;
}

