
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "pt_error.h"

static struct {
	const enum pt_error err;
	const char* const str;
} pt_error_map[PT_LAST] = {
	{ PT_SUCCESS, "Success" },
	{ PT_EFQFULL, "The function queue was full" },
	{ PT_EFQEMPTY, "The function queue was empty" },
	{ PT_EPML, "An error occurred while locking the mutex,"
		"check errno for more information" },
	{ PT_EPMTL, "An error occurred while locking the mutex without"
		"blocking, check errno for more information" },
	{ PT_EPMU, "An error occurred while unlocking the mutex,"
		"check errno for more information" },
	{ PT_EPMAI, "An error occurred while initializing the mutex"
		"attributes" },
	{ PT_EPMD, "An error occurred while unlocking the mutex,"
		"check errno for more information" },
	{ PT_EPO, "An error occurred while unlocking the mutex,"
		"check errno for more information" },
	{ PT_EPC, "An error occurred while creating a thread,"
		"check errno for more information" },
	{ PT_EMALLOC, "An error occurred while unlocking the mutex,"
		"check errno for more information" },
};

static char*
pt_strncpy(char* dst, const char* src, size_t n)
{
	size_t len = strlen(src);

	if(len > n)
		len = n;

	if(len > 0) {
		(void) memcpy(dst, src, len - 1);
		dst[len] = '\0';
	}

	return dst;
}

int
pt_strerror_r(enum pt_error err, char* buf, size_t len)
{
	int ret = 0;

	if(len > 0) {
		if(err < PT_LAST && err >= PT_SUCCESS) {
			assert(err == pt_error_map[err].err);
			(void) pt_strncpy(buf, pt_error_map[err].str, len);

			if(strlen(pt_error_map[err].str) >= len) {
				errno = ERANGE;
				ret = 1;
			}
		} else {
			errno = EINVAL;
			ret = 1;
		}
	}

	return ret;
}

