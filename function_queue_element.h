
#ifndef FUCNTION_QUEUE_ELEMENT_H
#define FUCNTION_QUEUE_ELEMENT_H

/*
 * This stucture holds a function pointer func and a corresponding
 * argument arg. Through this, a procedure can be "bound" to an argument
 * for when it is called.
 */
struct function_queue_element {
	void (* func)(void*);
	void* arg;
};


#endif

