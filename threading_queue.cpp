
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

#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <string>

#include "qtpool.h"
#include "qterror.h"

using namespace tq;

static const std::string error_prefix( "threading_queue: " );

static inline void
throw_runtime_error( std::string what )
{
	throw std::runtime_error( error_prefix + what );
}

static inline void
unhandled_errno( int err )
{
	throw_runtime_error( strerror( err ));
}

queue::
queue(qtpool_startup_info& tqsi)
{
	enum qterror ret = qtinit(this, &tqsi);

	if(ret == QTEERRNO) {
		unhandled_errno(ret);
	} else if(ret != QTSUCCESS) {
		char error_string[256];

		qtstrerror_r(ret, error_string, sizeof error_string);
		throw_runtime_error(error_string);
	}
}

queue::
~queue( void )
{ }

void
queue::
destroy(void)
{
	enum qterror ret = qtdestroy(this);

	if(ret != 0)
		unhandled_errno( ret );
}

void
queue::
start( void )
{
	int started = 0;
	enum qterror ret = qtstart(this, &started);

	if(ret == QTEERRNO) {
		unhandled_errno(ret);
	} else if(ret != QTSUCCESS) {
		char error_string[256];

		qtstrerror_r(ret, error_string, sizeof error_string);
		throw_runtime_error(error_string);
	}
}

void
queue::
stop( void )
{
	enum qterror ret = qtstop(this, 1);

	if(ret == QTEERRNO) {
		unhandled_errno(ret);
	} else if(ret != QTSUCCESS) {
		char error_string[256];

		qtstrerror_r(ret, error_string, sizeof error_string);
		throw_runtime_error(error_string);
	}
}

