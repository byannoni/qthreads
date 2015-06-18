
#ifndef PT_ERROR_H
#define PT_ERROR_H

enum pt_error {
	PT_SUCCESS,
	PT_EERRNO,
	PT_EFQFULL,
	PT_EFQEMPTY,
	PT_EPTMLOCK,
	PT_EPTMTRYLOCK,
	PT_EPTMUNLOCK,
	PT_EPTMAINIT,
	PT_EPTMDESTROY,
	PT_EPTONCE,
	PT_EPTCANCEL,
	PT_EMALLOC,

	PT_ELAST
};

int pt_strerror_r(enum pt_error, char*, size_t);

#endif

