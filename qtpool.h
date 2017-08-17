
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

struct qtstart_errors_info {
	int* errors;
	size_t current;
};

struct qtpool_startup_info {
	struct function_queue* fq;
	size_t max_threads;
};

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
#endif
#endif

