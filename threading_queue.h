
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

#include <pthread.h>

#include "function_queue.h"

struct tq_start_errors_info {
	int* errors;
	size_t current;
};

struct threading_queue_startup_info {
	struct function_queue* fq;
	size_t max_threads;
};

struct threading_queue {
	struct tq_start_errors_info start_errors;
	struct function_queue* fq;
	pthread_t* threads;
	size_t max_threads;
};

#ifdef __cplusplus
extern "C" {
#endif

enum pt_error tq_init(struct threading_queue*,
		struct threading_queue_startup_info* tqsi);
enum pt_error tq_destroy(struct threading_queue*);
enum pt_error tq_start(struct threading_queue*, int*);
enum pt_error tq_stop(struct threading_queue*, int);
enum pt_error tq_start_get_e(struct threading_queue*, size_t, int*);

#ifdef __cplusplus
}
#endif
#endif

