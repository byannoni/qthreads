
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

#ifndef FUNCTION_QUEUE_H
#define FUNCTION_QUEUE_H

#include <pthread.h>

#include "fq/indexed_array_queue.h"
#include "fq/linked_list_queue.h"
#include "function_queue_element.h"
#include "qterror.h"

/*
 * This contains the constants which describe the type and
 * implementation of a function queue. FQTYPE_LAST is not a real type,
 * but a marker of the final constant.
 */
enum fqtype {
	FQTYPE_IA, /* indexed array */
	FQTYPE_LL, /* linked list */

	FQTYPE_LAST /* not an actual type */
};

struct function_queue;

/*
 * This structure holds a dispatch table of procedures which correspond
 * to the functionality of a queue. The init member is for any
 * initialization which may be necessary for the queue. The destroy
 * member should clean up any resources which were in use. The push, pop
 * and peek procedures should provide their expected functionality.
 * The procedures which these members point to should not interact with
 * any member of the function queue object except the member queue.
 */
struct fqdispatchtable {
	enum qterror (* init)(struct function_queue*, unsigned);
	enum qterror (* destroy)(struct function_queue*);
	enum qterror (* push)(struct function_queue*, void (*)(void*),
			void*, int);
	enum qterror (* pop)(struct function_queue*,
			struct function_queue_element*, int);
	enum qterror (* peek)(struct function_queue*,
			struct function_queue_element*, int);
	enum qterror (* resize)(struct function_queue*, unsigned int, int);
};

struct function_queue {
	union fqvariant { /* union types of queue data */
		struct fqindexedarray ia; /* indexed array queue */
		struct fqlinkedlist ll; /* indexed array queue */
	} queue;
	/* table of procedures for manipulating the queue data */
	const struct fqdispatchtable* dispatchtable;
	/* lock for managing the thread safety of the queue data */
	pthread_mutex_t lock;
	/* condition variable for signaling full and empty events */
	pthread_cond_t wait;
	enum fqtype type; /* the type identifier of the queue */
	unsigned int max_elements; /* the maximum size of the queue */
	unsigned int size; /* the true size of the queue */
};

#ifdef __cplusplus
extern "C" {
#endif

enum qterror fqinit(struct function_queue*, enum fqtype, unsigned);
enum qterror fqdestroy(struct function_queue*);
enum qterror fqpush(struct function_queue*, void (*)(void*), void*, int);
enum qterror fqpop(struct function_queue*, struct function_queue_element*,
		int);
enum qterror fqpeek(struct function_queue*, struct function_queue_element*,
		int);
enum qterror fqisempty(struct function_queue*, int*);
enum qterror fqisfull(struct function_queue*, int*);
enum qterror fqresize(struct function_queue*, unsigned int, int);

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

		void destroy(void);
		void push( element&, int );
		element pop( int );
		element peek( int );
		bool is_empty( int );
		bool is_full( int );
	};
}
#endif
#endif

