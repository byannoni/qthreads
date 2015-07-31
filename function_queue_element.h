
#ifndef FUNCTION_QUEUE_ELEMENT_H
#define FUNCTION_QUEUE_ELEMENT_H

struct function_queue_element {
	void (* func)(void*);
	void* arg;
};

#endif

