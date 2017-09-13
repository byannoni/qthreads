
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

#include <assert.h>
#include <string.h>
#include <errno.h>

#include "qterror.h"

/*
 * An array to contain valid error codes and corresponding descriptive
 * strings. The member err contains the actual error code. The member
 * str is a pointer to a constant string which describes the error.
 */
static struct {
	const enum qterror err;
	const char* const str;
} qterror_map[QTELAST] = {
	{ QTSUCCESS, "Success" },
	{ QTEERRNO, "Check errno for more information" },
	{ QTEFQFULL, "The function queue was full" },
	{ QTEFQEMPTY, "The function queue was empty" },
	{ QTEPTMLOCK, "An error occurred while locking the mutex, check errno for more information" },
	{ QTEPTMTRYLOCK, "An error occurred while locking the mutex without blocking, check errno for more information" },
	{ QTEPTMUNLOCK, "An error occurred while unlocking the mutex, check errno for more information" },
	{ QTEPTMAINIT, "An error occurred while initializing the mutex attributes" },
	{ QTEPTMDESTROY, "An error occurred while destroying a mutex, check errno for more information" },
	{ QTEPTONCE, "An error occurred during dynamic initialization, check errno for more information" },
	{ QTEPTCREATE, "An error occurred while creating a thread, check errno for more information" },
	{ QTEMALLOC, "An error occurred while allocating memory, check errno for more information" },
	{ QTEPTCINIT, "An error occurred while initializing a condition variable, check errno for more information" },
	{ QTEPTCDESTROY, "An error occurred while destroying a condition variable, check errno for more information" },
	{ QTEINVALID, "An invalid value was encountered" },
	{ QTEPTMINIT, "An error occurred while initializing the mutex" },
};

/*
 * Copy a string from src to dst.
 *
 * This procedure copies at most n characters from the buffer pointed to
 * by src to the buffer pointed to by dst. The character after the last
 * character copied is set to NUL unless the index of that character
 * would be greater than n. In that case, the n-th character is set to
 * NUL. The string dst is properly null-terminated unless the string src
 * is empty or the variable n is 0. This procedure returns the value of
 * dst. Neither the value of dst nor the value of src may be NULL. The
 * buffer pointed to by src must be null-terminated.
 */
static char*
qtstrncpy(char* dst, const char* src, size_t n)
{
	size_t len = 0;

	assert(dst != NULL);
	assert(src != NULL);
	len = strlen(src);

	if(len > n)
		len = n;

	if(len > 0) {
		(void) memcpy(dst, src, len);
		dst[len] = '\0';
	}

	return dst;
}

/*
 * Retrieve a string describing the given error code.
 *
 * This procedure copies at most len-1 bytes from the error description
 * string in qterror_map which corresponds to the given error code into
 * the buffer pointed to by buf. This procedure always succeeds and
 * returns 0. The value of err must be a valid qterror value. The value
 * of buf must not be NULL. If the buffer pointed to by buf is modified
 * then it will be null-terminated.
 */
int
qtstrerror_r(enum qterror err, char* buf, size_t len)
{
	if(len > 0) {
		if(err >= QTELAST || err < QTSUCCESS) {
			errno = EINVAL;
			return 1;
		}

		assert(err == qterror_map[err].err);
		(void) qtstrncpy(buf, qterror_map[err].str, len - 1);

		if(strlen(qterror_map[err].str) >= len - 2) {
			errno = ERANGE;
			return 1;
		}
	}

	return 0;
}

