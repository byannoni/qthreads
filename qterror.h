
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

#ifndef QTERROR_H
#define QTERROR_H

/* An enum of error codes for use in qt* procedures */
enum qterror {
	QTSUCCESS, /* success */
	QTEERRNO, /* an error occurred indicated by errno */
	QTEFQFULL, /* the function queue was full */
	QTEFQEMPTY, /* the function queue was empty */
	QTEPTMLOCK, /* an error occurred in pthread_mutex_lock */
	QTEPTMTRYLOCK, /* an error occurred in pthread_mutex_trylock */
	QTEPTMUNLOCK, /* an error occurred in pthread_mutex_unlock */
	QTEPTMAINIT, /* an error occurred in pthread_mutexattr_init */
	QTEPTMDESTROY, /* an error occurred in pthread_mutex_destroy */
	QTEPTONCE, /* an error occurred in pthread_once */
	QTEPTCREATE, /* an error occurred in pthread_create */
	QTEMALLOC, /* an error occurred in malloc */
	QTEPTCINIT, /* an error occurred in pthread_cond_init */
	QTEPTCDESTROY, /* an error occurred in pthread_cond_destroy */
	QTEINVALID, /* an invalid value was encountered */
	QTEPTMINIT, /*an error occurred in pthread_mutex_init */

	QTELAST /* the last error code; not a valid error */
};

#ifdef __cplusplus
extern "C" {
#endif
int qtstrerror_r(enum qterror, char*, size_t);
#ifdef __cplusplus
}
#endif
#endif

