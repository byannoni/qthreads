
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

#ifndef INDEXED_ARRAY_QUEUE_H
#define INDEXED_ARRAY_QUEUE_H

#include <pthread.h>

#include "../function_queue.h"
#include "../qterror.h"

struct fqindexedarray {
	struct function_queue_element* elements;
	pthread_mutex_t lock;
	pthread_cond_t wait;
	unsigned int front;
	unsigned int back;
};

extern const struct fqdispatchtable fqdispatchtableia;

#endif

