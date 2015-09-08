
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

#include "fq/indexed_array_queue.h"
#include "pt_error.h"

enum fqtype {
	FQTYPE_IA, /* indexed array */
	FQTYPE_LAST /* not an actual type */
};

struct function_queue;

struct function_queue_element {
	void (* func)(void*);
	void* arg;
};

struct fqdispatchtable {
	enum pt_error (* init)(struct function_queue*, unsigned);
	enum pt_error (* destroy)(struct function_queue*);
	enum pt_error (* push)(struct function_queue*, void (*)(void*),
			void*, int);
	enum pt_error (* pop)(struct function_queue*,
			struct function_queue_element*, int);
	enum pt_error (* peek)(struct function_queue*,
			struct function_queue_element*, int);
};

struct function_queue {
	union fqvariant {
		struct fqindexedarray ia;
	} queue;
	const struct fqdispatchtable* dispatchtable;
	pthread_mutex_t lock;
	pthread_cond_t wait;
	enum fqtype type;
	unsigned int max_elements;
	unsigned int size;
};

#ifdef __cplusplus
extern "C" {
#endif

enum pt_error fqinit(struct function_queue*, enum fqtype, unsigned);
enum pt_error fqdestroy(struct function_queue*);
enum pt_error fqpush(struct function_queue*, void (*)(void*), void*, int);
enum pt_error fqpop(struct function_queue*, struct function_queue_element*,
		int);
enum pt_error fqpeek(struct function_queue*, struct function_queue_element*,
		int);
enum pt_error fqisempty(struct function_queue*, int*);
enum pt_error fqisfull(struct function_queue*, int*);

#ifdef __cplusplus
}
#endif
#endif

