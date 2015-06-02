
#ifndef THREADING_QUEUE_H
#define THREADING_QUEUE_H

#include "function_queue.h"

#include <unistd.h>

struct threading_queue_startup_info {
	unsigned max_threads;
	struct function_queue* fq;
	useconds_t delay;
};

struct threading_queue {
	unsigned max_threads;
	useconds_t delay;
	pthread_t* threads;
	struct function_queue* fq;
};

#ifdef __cplusplus
extern "C" {
#endif

int tq_init( struct threading_queue*,
		struct threading_queue_startup_info* tqsi );
int tq_destroy( struct threading_queue* );
int tq_start( struct threading_queue* );
int tq_stop( struct threading_queue* );

#ifdef __cplusplus
}

namespace tq {
	class queue : public ::threading_queue {
	public:
		queue( threading_queue_startup_info& );
		~queue( void );

		void start( void );
		void stop( void );
	};
}
#endif
#endif
#endif

