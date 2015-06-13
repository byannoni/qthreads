
#ifndef PT_ERROR_H
#define PT_ERROR_H

enum pt_error {
	PT_SUCCESS,
	PT_LAST
};

int pt_strerror_r(enum pt_error, char*, size_t);

#endif

