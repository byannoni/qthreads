
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

int threading_queue_init( struct threading_queue*,
		struct threading_queue_startup_info tqsi[static 1] );
int threading_queue_destroy( struct threading_queue* );
int start( struct threading_queue* );
int stop( struct threading_queue* );

#ifdef __cplusplus
}
#endif

