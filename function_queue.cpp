
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <string>

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
queue( unsigned max_elements )
{
	int ret = fq_init( this, max_elements );

	if( ret ) {
		if( ret == EMUTEXATTR_INIT ) {
			throw_runtime_error( "error initializing mutex" );
		} else if( ret == EMUTEXATTR_SETTYPE ) {
			throw_runtime_error( "could not make mutex recursive" );
		} else {
			unhandled_errno( ret );
		}
	}
}

queue::
~queue( void )
{
	/* TODO add exception handling */
	fq_destroy( this );
}

void
queue::
push( element& e, int block )
{
	int ret = fq_push( this, e, block );

	if( ret == ERANGE ) {
		throw std::overflow_error( "function_queue: overflowed" );
	} else if( ret ) {
		unhandled_errno( ret );
	}
}

element
queue::
pop( int block )
{
	element e;
	/* TODO add exception handling */
	fq_pop( this, &e, block );
	return e;
}

element
queue::
peek( int block )
{
	element e;
	/* TODO add exception handling */
	fq_peek( this, &e, block );
	return e;
}

bool
queue::
is_empty( int block )
{
	/* TODO add exception handling */
	return fq_is_empty( this, block );
}

bool
queue::
is_full( int block )
{
	/* TODO add exception handling */
	return fq_is_full( this, block );
}

