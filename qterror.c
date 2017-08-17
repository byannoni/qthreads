
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

#include "qterror.h"

static struct {
	const enum qterror err;
	const char* const str;
} qterror_map[QTELAST] = {
	{ QTSUCCESS, "Success" },
	{ QTEERRNO, "Check errno for more information" },
	{ QTEFQFULL, "The function queue was full" },
	{ QTEFQEMPTY, "The function queue was empty" },
	{ QTEPTMLOCK, "An error occurred while locking the mutex,"
		"check errno for more information" },
	{ QTEPTMTRYLOCK, "An error occurred while locking the mutex without"
		"blocking, check errno for more information" },
	{ QTEPTMUNLOCK, "An error occurred while unlocking the mutex,"
		"check errno for more information" },
	{ QTEPTMAINIT, "An error occurred while initializing the mutex"
		"attributes" },
	{ QTEPTMDESTROY, "An error occurred while destroying a mutex,"
		"check errno for more information" },
	{ QTEPTONCE, "An error occurred during dynamic initialization," 
		"check errno for more information" },
	{ QTEPTCREATE, "An error occurred while creating a thread,"
		"check errno for more information" },
	{ QTEMALLOC, "An error occurred while allocating memory,"
		"check errno for more information" },
	{ QTEPTCINIT, "An error occurred while initializing a condition"
		"variable, check errno for more information" },
	{ QTEPTCDESTROY, "An error occurred while destroying a condition"
		"variable, check errno for more information" },
	{ QTEINVALID, "An invalid value was encountered" },
	{ QTEPTMINIT, "An error occurred while initializing the mutex" },
};

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

