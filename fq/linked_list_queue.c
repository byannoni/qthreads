
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

#include "../function_queue.h"
#include "linked_list_queue.h"
#include "../qterror.h"

static enum qterror fqinitll(struct function_queue*, unsigned);
static enum qterror fqdestroyll(struct function_queue*);
static enum qterror fqpushll(struct function_queue*, void (*)(void*), void*,
		int);
static enum qterror fqpopll(struct function_queue*,
		struct function_queue_element*, int);
static enum qterror fqpeekll(struct function_queue*,
		struct function_queue_element*, int);

/*
 * This is the function dispatch table for manipulating the queue in an
 * implementation-agnostic way.
 */
const struct fqdispatchtable fqdispatchtablell = {
	fqinitll,
	fqdestroyll,
	fqpushll,
	fqpopll,
	fqpeekll
};

/*
 * This procedure initializes the queue. The value of max_elements is
 * the maximum number of elements which the queue will store. This
 * procedure always succeeds. The value of q must not be NULL.
 */
static enum qterror
fqinitll(struct function_queue* q, unsigned max_elements)
{
	/* suppress unused variable warning */
	(void) max_elements;

	assert(q != NULL);
	q->queue.ll.head = NULL;
	q->queue.ll.tail = NULL;
	q->size = 0;
	q->max_elements = max_elements;

	return QTSUCCESS;
}

/*
 * This procedure destroys the given queue. The memory for elements in
 * the queue is freed. An attempt to use the object after it has been
 * destoyed results in undefined behavior. This procedure always
 * succeeds. The value of q must not be NULL.
 */
static enum qterror
fqdestroyll(struct function_queue* q)
{
	struct fqellnode* tmp = NULL;

	assert(q != NULL);
	tmp = q->queue.ll.head;

	while(tmp != NULL) {
		struct fqellnode* next = q->queue.ll.head->next;

		free(tmp);
		tmp = next;
	}

	return QTSUCCESS;
}

/*
 * This procedure pushes the given function pointer onto the queue. The 
 * function pointer is stored with the given argument arg so the value
 * can be passed to it. This procedure does not block. It returns an
 * error code to indicate its status. The value of q must not be NULL.
 */
static enum qterror
fqpushll(struct function_queue* q, void (*func)(void*), void* arg, int block)
{
	struct function_queue_element e;
	struct fqellnode* new_node = NULL;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);
	e.func = func;
	e.arg = arg;
	new_node = malloc(sizeof(struct fqellnode));

	if(new_node == NULL)
		return QTEMALLOC;

	new_node->element = e;
	new_node->next = NULL;

	if(q->queue.ll.tail == NULL) {
		q->queue.ll.tail = new_node;
	} else {
		q->queue.ll.tail->next = new_node;
		q->queue.ll.tail = q->queue.ll.tail->next;
	}

	if(q->queue.ll.head == NULL)
		q->queue.ll.head = q->queue.ll.tail;

	return QTSUCCESS;
}

/*
 * This procedure pops a function pointer from the queue. The function
 * pointer and its information is stored in a function queue element.
 * The value of this function queue element is copied to the address
 * pointed to by the variable e and then removed from the queue. This
 * procedure does not block. This procedure always succeeds. The value
 * of q must not be NULL. The value of e must not be NULL.
 */
static enum qterror
fqpopll(struct function_queue* q, struct function_queue_element* e, int block)
{
	struct fqellnode* tmp = NULL;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);
	assert(e != NULL);
	*e = q->queue.ll.head->element;
	tmp = q->queue.ll.head;
	q->queue.ll.head = q->queue.ll.head->next;
	free(tmp);

	if(q->queue.ll.head == NULL)
		q->queue.ll.tail = NULL;

	return QTSUCCESS;
}

/*
 * This procedure peeks at a function pointer from the queue. The
 * function pointer and its information is stored in a function queue
 * element. The value of this function queue element is copied to the
 * address pointed to by the variable e. This procedure does not block.
 * The procedure always succeeds. The value of q must not be NULL. The
 * value of e must not be NULL.
 */
static enum qterror
fqpeekll(struct function_queue* q, struct function_queue_element* e, int block)
{
	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);
	assert(e != NULL);
	*e = q->queue.ll.head->element;

	return QTSUCCESS;
}

