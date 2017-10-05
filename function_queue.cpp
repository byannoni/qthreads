
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

#include "qterror.h"
#include "function_queue.h"

using namespace fq;

static const std::string error_prefix( "function_queue: " );

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

element::
element( void )
{ }

element::
element( void (* function)( void* ), void* argument )
{
	func = function;
	arg = argument;
}

queue::
queue(unsigned maximum_elements)
{
	enum qterror ret = fqinit(this, FQTYPE_IA, maximum_elements);

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
destroy(void)
{
	enum qterror ret = fqdestroy(this);

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
push( element& e, int block )
{
	enum qterror ret = fqpush(this, e.func, e.arg, block);

	if(ret == QTEFQFULL) {
		throw std::overflow_error( "function_queue: overflow" );
	} else if(ret == QTEERRNO) {
		unhandled_errno(ret);
	} else if(ret != QTSUCCESS) {
		char error_string[256];

		qtstrerror_r(ret, error_string, sizeof error_string);
		throw_runtime_error(error_string);
	}
}

element
queue::
pop( int block )
{
	element e;
	enum qterror ret = fqpop( this, &e, block );

	if(ret == QTEFQEMPTY) {
		throw std::underflow_error( "function_queue: underflow" );
	} else if(ret == QTEERRNO) {
		unhandled_errno(ret);
	} else if(ret != QTSUCCESS) {
		char error_string[256];

		qtstrerror_r(ret, error_string, sizeof error_string);
		throw_runtime_error(error_string);
	}

	return e;
}

element
queue::
peek( int block )
{
	element e;
	enum qterror ret = fqpeek( this, &e, block );

	if(ret == ERANGE) {
		throw std::underflow_error( "function_queue: underflow" );
	} else if(ret == QTEERRNO) {
		unhandled_errno(ret);
	} else if(ret != QTSUCCESS) {
		char error_string[256];

		qtstrerror_r(ret, error_string, sizeof error_string);
		throw_runtime_error(error_string);
	}

	return e;
}

bool
queue::
is_empty( int block )
{
	int isempty = 0;
	enum qterror ret = fqisempty(this, &isempty);

	(void) block;

	if(ret == QTEERRNO) {
		unhandled_errno(ret);
	} else if(ret != QTSUCCESS) {
		char error_string[256];

		qtstrerror_r(ret, error_string, sizeof error_string);
		throw_runtime_error(error_string);
	}

	return isempty;
}

bool
queue::
is_full( int block )
{
	int isfull = 0;
	enum qterror ret = fqisfull(this, &isfull);

	(void) block;

	if(ret == QTEERRNO) {
		unhandled_errno(ret);
	} else if(ret != QTSUCCESS) {
		char error_string[256];

		qtstrerror_r(ret, error_string, sizeof error_string);
		throw_runtime_error(error_string);
	}

	return isfull;
}

