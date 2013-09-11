
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
	int ret = fq_destroy( this );

	if( ret ) {
		unhandled_errno( ret );
	}
}

void
queue::
push( element& e, int block )
{
	int ret = fq_push( this, e, block );

	if( ret == ERANGE ) {
		throw std::overflow_error( "function_queue: overflow" );
	} else if( ret ) {
		unhandled_errno( ret );
	}
}

element
queue::
pop( int block )
{
	element e;
	int ret = fq_pop( this, &e, block );

	if( ret == ERANGE ) {
		throw std::underflow_error( "function_queue: underflow" );
	} else if( ret ) {
		unhandled_errno( ret );
	}

	return e;
}

element
queue::
peek( int block )
{
	element e;
	int ret = fq_peek( this, &e, block );

	if( ret == ERANGE ) {
		throw std::underflow_error( "function_queue: underflow" );
	} else if( ret ) {
		unhandled_errno( ret );
	}

	return e;
}

bool
queue::
is_empty( int block )
{
	int ret = fq_is_empty( this, block );

	if( ret != 0 && ret != 1 ) {
		unhandled_errno( ret );
	}

	return ret;
}

bool
queue::
is_full( int block )
{
	int ret = fq_is_full( this, block );

	if( ret != 0 && ret != 1 ) {
		unhandled_errno( ret );
	}

	return ret;
}

