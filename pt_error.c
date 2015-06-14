
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "pt_error.h"

static struct {
	const enum pt_error err;
	const char* const str;
} pt_error_map[PT_LAST] = {
	{ PT_SUCCESS, "Success" },
};

static char*
pt_strncpy(char* dst, const char* src, size_t n)
{
	if(n > 0) {
		int len = strnlen(src, n);

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

