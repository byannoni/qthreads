
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

#ifndef THREADING_QUEUE_H
#define THREADING_QUEUE_H

#include <pthread.h>

#include "function_queue.h"

/*
 * This structure holds the list of errors which occurred durring the
 * thread start up segment of qtstart(). The member errors is a pointer
 * to the first elements of an array of errno values corresponding to
 * the start status of each thread. The member current is unused.
 */
struct qtstart_errors_info {
	int* errors;
	size_t current;
};

/*
 * This structure contains information for creating the thread pool. The
 * member fq is a pointer to the function queue to use for the pool. The
 * member max_threads the maximum number of threads to use in the pool.
 */
struct qtpool_startup_info {
	struct function_queue* fq;
	size_t max_threads;
};

/*
 * This structure holds the actual pool information and data. The member
 * start_errors holds information about the errors which occurred while
 * the threads were starting. The member fq points to the function queue
 * structure which the pool uses. The member threads holds the address
 * of the array of threads which are used in the pool. The member
 * max_threads is the maximum number of threads which will be started
 * for the pool.
 */
struct qtpool {
	struct qtstart_errors_info start_errors;
	struct function_queue* fq;
	pthread_t* threads;
	size_t max_threads;
};

#ifdef __cplusplus
extern "C" {
#endif

enum qterror qtinit(struct qtpool*,
		struct qtpool_startup_info* tqsi);
enum qterror qtdestroy(struct qtpool*);
enum qterror qtstart(struct qtpool*, int*);
enum qterror qtstop(struct qtpool*, int);
enum qterror qtstart_get_e(struct qtpool*, size_t, int*);

#ifdef __cplusplus
}

namespace tq {
	class queue : public ::qtpool {
	public:
		queue(qtpool_startup_info&);
		~queue( void );

		void destroy(void);
		void start( void );
		void stop( void );
	};
}
#endif
#endif

