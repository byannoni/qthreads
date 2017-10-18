
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

#include "../function_queue_element.h"
#include "../function_queue.h"
#include "linked_list_queue.h"
#include "../qterror.h"

static enum qterror fqinitll(union fqvariant*, unsigned);
static enum qterror fqdestroyll(union fqvariant*);
static enum qterror fqpushll(union fqvariant*, void (*)(void*), void*, int);
static enum qterror fqpopll(union fqvariant*, struct function_queue_element*,
		int);
static enum qterror fqpeekll(union fqvariant*, struct function_queue_element*,
		int);
static enum qterror fqresizell(union fqvariant*, unsigned, int);
static enum qterror fqisemptyll(union fqvariant*, int*, int);
static enum qterror fqisfullll(union fqvariant*, int*, int);

static void fqellnode_trunc(struct fqellnode*);
/*
 * This is the function dispatch table for manipulating the queue in an
 * implementation-agnostic way.
 */
const struct fqdispatchtable fqdispatchtablell = {
	fqinitll,
	fqdestroyll,
	fqpushll,
	fqpopll,
	fqpeekll,
	fqresizell,
	fqisemptyll,
	fqisfullll,
};

/*
 * This procedure initializes the queue. The value of max_elements is
 * the maximum number of elements which the queue will store. This
 * procedure always succeeds. The value of q must not be NULL.
 */
static enum qterror
fqinitll(union fqvariant* q, unsigned max_elements)
{
	/* suppress unused variable warning */
	(void) max_elements;

	assert(q != NULL);
	q->ll.head = NULL;
	q->ll.tail = NULL;
	q->ll.size = 0;
	q->ll.max_size = max_elements;

	return QTSUCCESS;
}

/*
 * This procedure destroys the given queue. The memory for elements in
 * the queue is freed. An attempt to use the object after it has been
 * destoyed results in undefined behavior. This procedure always
 * succeeds. The value of q must not be NULL.
 */
static enum qterror
fqdestroyll(union fqvariant* q)
{
	assert(q != NULL);
	fqellnode_trunc(q->ll.head);
	return QTSUCCESS;
}

/*
 * This procedure pushes the given function pointer onto the queue. The
 * function pointer is stored with the given argument arg so the value
 * can be passed to it. This procedure does not block. It returns an
 * error code to indicate its status. The value of q must not be NULL.
 */
static enum qterror
fqpushll(union fqvariant* q, void (*func)(void*), void* arg, int block)
{
	struct function_queue_element e;
	struct fqellnode* new_node = NULL;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);

	if(q->ll.size >= q->ll.max_size)
		return QTEFQFULL;

	e.func = func;
	e.arg = arg;
	new_node = malloc(sizeof(struct fqellnode));

	if(new_node == NULL)
		return QTEMALLOC;

	new_node->element = e;
	new_node->next = NULL;
	++q->ll.size;

	if(q->ll.tail == NULL) {
		q->ll.tail = new_node;
	} else {
		q->ll.tail->next = new_node;
		q->ll.tail = q->ll.tail->next;
	}

	if(q->ll.head == NULL)
		q->ll.head = q->ll.tail;

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
fqpopll(union fqvariant* q, struct function_queue_element* e, int block)
{
	struct fqellnode* tmp = NULL;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);
	assert(e != NULL);

	if(q->ll.size == 0)
		return QTEFQEMPTY;

	--q->ll.size;
	*e = q->ll.head->element;
	tmp = q->ll.head;
	q->ll.head = q->ll.head->next;
	free(tmp);

	if(q->ll.head == NULL)
		q->ll.tail = NULL;

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
fqpeekll(union fqvariant* q, struct function_queue_element* e, int block)
{
	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);
	assert(e != NULL);

	if(q->ll.size == 0)
		return QTEFQEMPTY;

	*e = q->ll.head->element;

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
fqresizell(union fqvariant* q, unsigned int len, int block)
{
	struct fqellnode* tmp = NULL;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);

	q->ll.max_size = len;

	if(len >= q->ll.size)
		return QTSUCCESS;

	tmp = q->ll.head;
	q->ll.size = len;

	while(len-- > 0 && tmp != NULL) {
		tmp = tmp->next;
	}

	assert(tmp != NULL); /* this should never be possible */
	fqellnode_trunc(tmp->next);

	if(q->ll.size > 0) {
		tmp->next = NULL;
		q->ll.tail = tmp;
	} else {
		q->ll.head = NULL;
		q->ll.tail = NULL;
	}

	return QTSUCCESS;
}

/*
 * This procedure checks if the given queue is empty. It sets the value
 * at the address pointed to by isempty to 0 if the queue is empty.
 * Otherwise, it sets the value pointed to by isempty to non-zero. This
 * procedure returns an error code indicating its status. The value of q
 * must not be NULL. The value of isempty must not be NULL.
 */
static enum qterror
fqisemptyll(union fqvariant* q, int* isempty, int block)
{
	(void) block;

	assert(q != NULL);
	assert(isempty != NULL);
	*isempty = q->ll.size == 0;
	return QTSUCCESS;
}

/*
 * This procedure checks if the given queue is full. It sets the value
 * at the address pointed to by isfull to 0 if the queue is full.
 * Otherwise, it sets the value pointed to by isfull to non-zero. This
 * procedure returns an error code indicating its status. The value of q
 * must not be NULL. The value of isfull must not be NULL.
 */
static enum qterror
fqisfullll(union fqvariant* q, int* isfull, int block)
{
	(void) block;

	assert(q != NULL);
	assert(isfull != NULL);
	*isfull = q->ll.size >= q->ll.max_size;
	return QTSUCCESS;
}

static void
fqellnode_trunc(struct fqellnode* node)
{
	while(node != NULL) {
		struct fqellnode* next = node->next;

		free(node);
		node = next;
	}
}

