
#ifndef FUNCTION_QUEUE_H
#define FUNCTION_QUEUE_H

#include <pthread.h>

#ifdef __cplusplus
#include <stdexcept>
#endif

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
	EMUTEXATTR_INIT = ~-2,
	EMUTEXATTR_SETTYPE = ~-3
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

namespace fq {
	/*
	 * TODO
	 * Create exception class
	 */

	class element : public ::function_queue_element {
	public:
		element( void );
		element( void (*)( void* ), void* = 0 );
	};

	class queue : public ::function_queue {
	public:
		queue( unsigned );
		~queue( void );
		void push( element&, int );
		element pop( int );
		element peek( int );
		bool is_empty( int );
		bool is_full( int );
	};
}

#endif
#endif
