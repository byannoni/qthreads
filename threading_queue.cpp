
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

#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <string>

#include "threading_queue.h"

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
queue( threading_queue_startup_info& tqsi )
{
	int ret = tq_init( this, &tqsi );

	if( ret ) {
		unhandled_errno( ret );
	}
}

queue::
~queue( void )
{
	int ret = tq_destroy( this );

	if( ret ) {
		unhandled_errno( ret );
	}
}

void
queue::
start( void )
{
	int ret = tq_start( this );

	if( ret != static_cast<int>( max_threads )) {
		throw_runtime_error( "could not start all threads" );
	}
}

void
queue::
stop( void )
{
	int ret = tq_stop( this );

	if( ret ) {
		unhandled_errno( ret );
	}
}

