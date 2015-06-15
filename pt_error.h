
#ifndef PT_ERROR_H
#define PT_ERROR_H

enum pt_error {
	PT_SUCCESS,
	PT_EERRNO,
	PT_EFQFULL,
	PT_EFQEMPTY,
	PT_EPML,
	PT_EPMTL,
	PT_EPMU,
	PT_EPMAI,
	PT_EPMD,
	PT_EPO,
	PT_EPC,
	PT_EMALLOC,

	PT_LAST
};

int pt_strerror_r(enum pt_error, char*, size_t);

#endif

