
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

