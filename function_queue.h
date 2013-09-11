
#ifndef FUNCTION_QUEUE_H
#define FUNCTION_QUEUE_H

#include <pthread.h>

struct function_queue_element {
	void (* func)(void*);
	void* arg;
};

struct function_queue {
	unsigned front;
	unsigned back;
	unsigned max_elements;
	unsigned size;
	struct function_queue_element* elements;
	pthread_mutex_t lock;
};

enum {
	EMUTEXATTR_INIT = ~ ((unsigned) -1 >> 1 ),
	EMUTEXATTR_SETTYPE = ~ ((unsigned) -1 >> 2 )
};

#ifdef __cplusplus
extern "C" {
#endif

int fq_init( struct function_queue*, unsigned );
int fq_destroy( struct function_queue* );
int fq_push( struct function_queue*, struct function_queue_element, int );
int fq_pop( struct function_queue*, struct function_queue_element*, int );
int fq_peek( struct function_queue*, struct function_queue_element*, int );
int fq_is_empty( struct function_queue*, int );
int fq_is_full( struct function_queue*, int );

#ifdef __cplusplus
}
#endif

#endif

