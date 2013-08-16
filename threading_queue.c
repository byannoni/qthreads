
#include "threading_queue.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

static void* get_and_run( struct threading_queue* tq )
{
	struct function_queue_element fqe;

	do {
		if( !pop( tq->fq, &fqe ))
			fqe.func( fqe.arg );

		usleep( tq->delay );
	} while( 1 );

	return 0;
}

int threading_queue_init( struct threading_queue* tq, unsigned max_threads,
				struct function_queue* fq, useconds_t delay )
{
	int ret = 0;
	tq->fq = fq;
	tq->max_threads = max_threads;
	tq->delay = delay;
	tq->threads = malloc( max_threads * sizeof( pthread_t ));

	if( !tq->threads )
		ret = ENOMEM;

	return ret;
}

int threading_queue_destroy( struct threading_queue* tq )
{
	return stop( tq );
}

int start( struct threading_queue* tq )
{
	int i = 0;
	int ret = 0;
	
	for( i = 0; i < tq->max_threads; ++i )
		if( !pthread_create( &tq->threads[i], 0, (void*(*)(void*))get_and_run, tq ))
			++ret;

	return ret;
}

int stop( struct threading_queue* tq )
{
	int i = 0;

	for( i = 0; i < tq->max_threads; ++i )
		pthread_cancel( tq->threads[i] );

	return 0;
}

