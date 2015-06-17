
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

#ifndef FUNCTION_QUEUE_H
#define FUNCTION_QUEUE_H

#include <pthread.h>
#include <semaphore.h>

struct function_queue_element {
	void (* func)(void*);
	void* arg;
};

struct function_queue {
	struct function_queue_element* elements;
	pthread_mutex_t lock;
	sem_t size;
	unsigned int front;
	unsigned int back;
	unsigned int max_elements;
};

/*
 * XXX
 * According to the C11 standard, this could conflict with implementation-
 * specific errno values. However, this case is very unlikely since all common
 * errno values are positive.
 */
enum {
	EMUTEXATTR_INIT = -1,
	EMUTEXATTR_SETTYPE = -2
};

#ifdef __cplusplus
extern "C" {
#endif

int fq_init(struct function_queue*, unsigned);
int fq_destroy(struct function_queue*);
int fq_push(struct function_queue*, struct function_queue_element, int);
int fq_pop(struct function_queue*, struct function_queue_element*, int);
int fq_peek(struct function_queue*, struct function_queue_element*, int);
int fq_is_empty(struct function_queue*, int);
int fq_is_full(struct function_queue*, int);

#ifdef __cplusplus
}
#endif

#endif

