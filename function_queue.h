
#ifndef FUNCTION_QUEUE_H
#define FUNCTION_QUEUE_H

#include <pthread.h>

struct function_queue_element {
	void (* func)(void*);
	void* arg;
};

struct function_queue {
	volatile unsigned front;
	volatile unsigned back;
	volatile unsigned max_elements;
	volatile struct function_queue_element* elements;
	pthread_mutex_t lock;
};

#ifdef __cplusplus
extern "C" {
#endif

int function_queue_init( struct function_queue*, unsigned );
int function_queue_destroy( struct function_queue* );
int push( struct function_queue*, struct function_queue_element );
int pop( struct function_queue*, struct function_queue_element* );
int peek( struct function_queue*, struct function_queue_element* );
int is_empty( struct function_queue* );
int is_full( struct function_queue* );
int resize( struct function_queue*, unsigned );

#ifdef __cplusplus
}
#endif

#endif

