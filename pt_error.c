
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

#include <assert.h>
#include <string.h>
#include <errno.h>

#include "pt_error.h"

static struct {
	const enum pt_error err;
	const char* const str;
} pt_error_map[PT_ELAST] = {
	{ PT_SUCCESS, "Success" },
	{ PT_EERRNO, "Check errno for more information" },
	{ PT_EFQFULL, "The function queue was full" },
	{ PT_EFQEMPTY, "The function queue was empty" },
	{ PT_EPTMLOCK, "An error occurred while locking the mutex,"
		"check errno for more information" },
	{ PT_EPTMTRYLOCK, "An error occurred while locking the mutex without"
		"blocking, check errno for more information" },
	{ PT_EPTMUNLOCK, "An error occurred while unlocking the mutex,"
		"check errno for more information" },
	{ PT_EPTMAINIT, "An error occurred while initializing the mutex"
		"attributes" },
	{ PT_EPTMDESTROY, "An error occurred while unlocking a mutex,"
		"check errno for more information" },
	{ PT_EPTONCE, "An error occurred during dynamic initialization," 
		"check errno for more information" },
	{ PT_EPTCREATE, "An error occurred while creating a thread,"
		"check errno for more information" },
	{ PT_EMALLOC, "An error occurred while allocating memory,"
		"check errno for more information" },
};

static char*
pt_strncpy(char* dst, const char* src, size_t n)
{
	size_t len = strlen(src);

	assert(dst != NULL);
	assert(src != NULL);

	if(len > n)
		len = n;

	if(len > 0) {
		(void) memcpy(dst, src, len);
		dst[len] = '\0';
	}

	return dst;
}

int
pt_strerror_r(enum pt_error err, char* buf, size_t len)
{
	int ret = 0;

	if(len > 0) {
		if(err < PT_ELAST && err >= PT_SUCCESS) {
			assert(err == pt_error_map[err].err);
			(void) pt_strncpy(buf, pt_error_map[err].str, len - 1);

			if(strlen(pt_error_map[err].str) >= len - 2) {
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

