
/*
 * Copyright 2015 Brandon Yannoni
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

#include "function_queue.h"

#include <unistd.h>

struct threading_queue_startup_info {
	struct function_queue* fq;
	useconds_t delay;
	unsigned int max_threads;
};

struct threading_queue {
	struct function_queue* fq;
	pthread_t* threads;
	useconds_t delay;
	unsigned int max_threads;
};

#ifdef __cplusplus
extern "C" {
#endif

int tq_init(struct threading_queue*,
		struct threading_queue_startup_info* tqsi);
int tq_destroy(struct threading_queue*);
int tq_start(struct threading_queue*);
int tq_stop(struct threading_queue*);

#ifdef __cplusplus
}

namespace tq {
	class queue : public ::threading_queue {
	public:
		queue( threading_queue_startup_info& );
		~queue( void );

		void destroy(void);
		void start( void );
		void stop( void );
	};
}
#endif
#endif

