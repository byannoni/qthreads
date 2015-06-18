
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
	PT_EPTCREATE,
	PT_EMALLOC,

	PT_ELAST
};

int pt_strerror_r(enum pt_error, char*, size_t);

#endif

