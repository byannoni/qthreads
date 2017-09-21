
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

#ifndef LINKED_LIST_QUEUE_H
#define LINKED_LIST_QUEUE_H

#include <pthread.h>

#include "../function_queue.h"
#include "../qterror.h"

/*
 * This structure is a linked list node structure for function queue
 * elements.
 */
struct fqellnode {
	struct function_queue_element element; /* the element value */
	struct fqellnode* next; /* the address of the next node */
};

/*
 * This structure is used to store the function queue elements and any
 * persistant data necessary for the manipulation procedures.
 */
struct fqlinkedlist {
	struct fqellnode* head; /* a pointer to the head of the list */
	struct fqellnode* tail; /* a pointer to the tail of the list */
	size_t size; /* current number of elements */
};

extern const struct fqdispatchtable fqdispatchtablell;

#endif

