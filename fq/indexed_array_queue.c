
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

#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "../function_queue.h"
#include "indexed_array_queue.h"
#include "../pt_error.h"

static enum pt_error fqinitia(struct function_queue*, unsigned);
static enum pt_error fqdestroyia(struct function_queue*);
static enum pt_error fqpushia(struct function_queue*, void (*)(void*), void*,
		int);
static enum pt_error fqpopia(struct function_queue*,
		struct function_queue_element*, int);
static enum pt_error fqpeekia(struct function_queue*,
		struct function_queue_element*, int);

const struct fqdispatchtable fqdispatchtableia = {
	fqinitia,
	fqdestroyia,
	fqpushia,
	fqpopia,
	fqpeekia
};

static enum pt_error
fqinitia(struct function_queue* q, unsigned max_elements)
{
	enum pt_error ret = PT_SUCCESS;

	/* suppress unused variable warning */
	(void) max_elements;

	assert(q != NULL);
	q->queue.ia.front = 0;
	q->queue.ia.back = 0;
	q->queue.ia.elements = malloc(q->max_elements *
			sizeof(*q->queue.ia.elements));

	if(q->queue.ia.elements == NULL)
		ret = PT_EMALLOC;

	return ret;
}

static enum pt_error
fqdestroyia(struct function_queue* q)
{
	enum pt_error ret = PT_SUCCESS;

	assert(q != NULL);
	free((struct function_queue_element*) q->queue.ia.elements);
	q->queue.ia.elements = NULL;

	return ret;
}

static enum pt_error
fqpushia(struct function_queue* q, void (*func)(void*), void* arg, int block)
{
	struct function_queue_element e;
	enum pt_error ret = PT_SUCCESS;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);
	e.func = func;
	e.arg = arg;

	if(++q->queue.ia.back == q->max_elements)
		q->queue.ia.back = 0;

	q->queue.ia.elements[q->queue.ia.back] = e;
	return ret;
}

static enum pt_error
fqpopia(struct function_queue* q, struct function_queue_element* e, int block)
{
	enum pt_error ret = PT_SUCCESS;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);
	assert(e != NULL);

	if(++q->queue.ia.front == q->max_elements)
		q->queue.ia.front = 0;

	*e = q->queue.ia.elements[q->queue.ia.front];
	return ret;
}

static enum pt_error
fqpeekia(struct function_queue* q, struct function_queue_element* e, int block)
{
	enum pt_error ret = PT_SUCCESS;
	unsigned int tmp = q->queue.ia.front + 1;

	/* suppress unused variable warning */
	(void) block;

	assert(q != NULL);
	assert(e != NULL);

	if(tmp == q->max_elements)
		tmp = 0;

	*e = q->queue.ia.elements[tmp];
	return ret;
}
