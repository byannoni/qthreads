
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

#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

#include "../function_queue_element.h"
#include "../function_queue.h"
#include "indexed_array_queue.h"
#include "../qterror.h"

static enum qterror fqinitia(struct function_queue*, unsigned);
static enum qterror fqdestroyia(struct function_queue*);
static enum qterror fqpushia(struct function_queue*, void (*)(void*), void*,
		int);
static enum qterror fqpopia(struct function_queue*,
		struct function_queue_element*, int);
static enum qterror fqpeekia(struct function_queue*,
		struct function_queue_element*, int);
static enum qterror fqresizeia(struct function_queue*, unsigned int, int);
static enum qterror fqisemptyia(struct function_queue*, int*, int);
static enum qterror fqisfullia(struct function_queue*, int*, int);
static unsigned int inc_and_wrap_index(unsigned int, unsigned int);

/*
 * This is the function dispatch table for manipulating the queue in an
 * implementation-agnostic way.
 */
const struct fqdispatchtable fqdispatchtableia = {
	fqinitia,
	fqdestroyia,
	fqpushia,
	fqpopia,
	fqpeekia,
	fqresizeia,
	fqisemptyia,
	fqisfullia,
};

/*
 * This procedure initializes the queue. The value of max_elements is
 * the maximum number of elements which will fit in the queue. Memory is
 * allocated for exactly that many elements. This procedure returns an
 * error code indicating its status. The value of q must not be NULL.
 */
static enum qterror
fqinitia(struct function_queue* q, unsigned max_elements)
{
	assert(q != NULL);
	q->queue.ia.front = 0;
	q->queue.ia.back = 0;
	q->queue.ia.size = 0;
	q->queue.ia.max_size = max_elements;
	q->queue.ia.elements = malloc(q->queue.ia.max_size *
			sizeof(*q->queue.ia.elements));

	if(q->queue.ia.elements == NULL)
		return QTEMALLOC;

	return QTSUCCESS;
}

/*
 * This procedure destroys the given queue. The memory for elements in
 * the queue is freed. An attempt to use the object after it has been
 * destroyed results in undefined behavior. This procedure returns an
 * error code indicating its status. The value of q must not be NULL.
 */
static enum qterror
fqdestroyia(struct function_queue* q)
{
	assert(q != NULL);
	free((struct function_queue_element*) q->queue.ia.elements);
	q->queue.ia.elements = NULL;

	return QTSUCCESS;
}

/*
 * This procedure pushes the given function pointer onto the queue. The
 * function pointer is stored with the given argument arg so the value
 * can be passed to it. This procedure does not block. It returns an
 * error code to indicate its status. The value of q must not be NULL.
 */
static enum qterror
fqpushia(struct function_queue* q, void (*func)(void*), void* arg, int block)
{
	struct function_queue_element e;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);

	if(q->queue.ia.size > q->queue.ia.max_size)
		return QTEFQFULL;

	e.func = func;
	e.arg = arg;
	q->queue.ia.back = inc_and_wrap_index(q->queue.ia.back,
			q->queue.ia.max_size);
	q->queue.ia.elements[q->queue.ia.back] = e;
	++q->queue.ia.size;
	return QTSUCCESS;
}

/*
 * This procedure pops a function pointer from the queue. The function
 * pointer and its information is stored in a function queue element.
 * The value of this function queue element is copied to the address
 * pointed to by the variable e and then removed from the queue. This
 * procedure does not block. It returns an error code to indicate its
 * status. The value of q must not be NULL. The value of e must not be
 * NULL.
 */
static enum qterror
fqpopia(struct function_queue* q, struct function_queue_element* e, int block)
{
	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);
	assert(e != NULL);

	if(q->queue.ia.size == 0)
		return QTEFQEMPTY;

	q->queue.ia.front = inc_and_wrap_index(q->queue.ia.front,
			q->queue.ia.max_size);
	*e = q->queue.ia.elements[q->queue.ia.front];
	--q->queue.ia.size;
	return QTSUCCESS;
}

/*
 * This procedure peeks at a function pointer from the queue. The
 * function pointer and its information is stored in a function queue
 * element. The value of this function queue element is copied to the
 * address pointed to by the variable e. This procedure does not block.
 * The procedure returns an error code to indicate its status. The value
 * of q must not be NULL. The value of e must not be NULL.
 */
static enum qterror
fqpeekia(struct function_queue* q, struct function_queue_element* e, int block)
{
	unsigned int tmp = 0;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);
	assert(e != NULL);
	tmp = inc_and_wrap_index(q->queue.ia.front, q->queue.ia.max_size);
	*e = q->queue.ia.elements[tmp];
	return QTSUCCESS;
}

/*
 * This procedure changes the maximum number of elements allowed in the
 * queue. It reallocates the elements array memory based on the new
 * maximum value len. This procedure does not block. If the new length
 * is not enough to store all the elements in the queue, the least
 * recently added elements are removed. This procedure returns an error
 * code indicating its status. The value of q must not be NULL.
 */
static enum qterror
fqresizeia(struct function_queue* q, unsigned int len, int block)
{
	struct function_queue_element* new_array = NULL;
	struct function_queue_element* old_array = NULL;
	unsigned int real_front = 0;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);

	if(len == q->queue.ia.size)
		return QTSUCCESS;

	new_array = malloc(len * sizeof(*new_array));

	if(new_array == NULL)
		return QTEMALLOC;

	real_front = inc_and_wrap_index(q->queue.ia.front,
			q->queue.ia.max_size);

	if(q->queue.ia.back >= q->queue.ia.front) {
		memcpy(new_array, &q->queue.ia.elements[real_front],
				len * sizeof(*new_array));
	} else if(q->queue.ia.back < q->queue.ia.front) {
		size_t num_elements_to_copy1 =
				len - q->queue.ia.front;
		size_t num_elements_to_copy2 = len - num_elements_to_copy1;

		memcpy(new_array, &q->queue.ia.elements[real_front],
				num_elements_to_copy1 * sizeof(*new_array));
		memcpy(&new_array[num_elements_to_copy1 + 1],
				q->queue.ia.elements,
				num_elements_to_copy2 * sizeof(*new_array));
	}

	if(q->queue.ia.size > len)
		q->queue.ia.size = len;

	q->queue.ia.front = len > 0 ? len - 1 : 0;
	q->queue.ia.back = len > 0 ? len - 1 : 0;
	old_array = q->queue.ia.elements;
	q->queue.ia.elements = new_array;
	q->queue.ia.max_size = len;
	free(old_array);
	return QTSUCCESS;
}

/*
 * This procedure checks if the given queue is empty. It sets the value
 * at the address pointed to by isempty to 0 if the queue is empty.
 * Otherwise, it sets the value pointed to by isempty to non-zero. This
 * procedure returns an error code indicating its status. The value of q
 * must not be NULL. The value of isempty must not be NULL.
 */
static enum qterror fqisemptyia(struct function_queue* q, int* isempty,
		int block)
{
	(void) block;

	assert(q != NULL);
	assert(isempty != NULL);
	*isempty = q->queue.ia.size == 0;
	return QTSUCCESS;
}

/*
 * This procedure checks if the given queue is full. It sets the value
 * at the address pointed to by isfull to 0 if the queue is full.
 * Otherwise, it sets the value pointed to by isfull to non-zero. This
 * procedure returns an error code indicating its status. The value of q
 * must not be NULL. The value of isfull must not be NULL.
 */
static enum qterror fqisfullia(struct function_queue* q, int* isfull, int block)
{
	(void) block;

	assert(q != NULL);
	assert(isfull != NULL);
	*isfull = q->queue.ia.size >= q->queue.ia.max_size;
	return QTSUCCESS;
}

/*
 * This procedure calculates the index of the the queue which results
 * from incrementing the given index and wrapping it appropriately with
 * the given maximum number of elements.  The value of index is the
 * index given by a position in the queue. The value of max is the value
 * of the max_elements member of the queue. This procedure returns the
 * index of the next element insertion point in the queue. The value of
 * max must be greater than or equal to the value of index.
 */
static unsigned int
inc_and_wrap_index(unsigned int index, unsigned int max)
{
	assert(index <= max);
	return index + 1 >= max ? 0 : index + 1;
}

