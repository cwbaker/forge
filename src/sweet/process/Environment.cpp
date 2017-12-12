//
// Environment.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "Environment.hpp"
#include <sweet/assert/assert.hpp>

using namespace sweet::process;

Environment::Environment( unsigned int values_reserve, unsigned int buffer_reserve )
: values_(),
  buffer_()
{
    values_.reserve( values_reserve );
    buffer_.reserve( buffer_reserve );
}

char* const* Environment::values() const
{
    return &values_[0];
}

const char* Environment::buffer() const
{
    return &buffer_[0];
}

void Environment::append( const char* key, const char* value )
{
    SWEET_ASSERT( key );
    SWEET_ASSERT( value );

    size_t key_length = strlen( key );
    size_t value_length = strlen( value );

    int key_start = buffer_.size();
    buffer_.resize( buffer_.size() + key_length + value_length + 2 );
    strcpy( &buffer_[key_start], key );
    buffer_[key_start + key_length] = '=';
    int value_start = key_start + key_length + 1;
    strcpy( &buffer_[value_start], value );
    buffer_[value_start + value_length + 0] = 0;

    values_.push_back( &buffer_[key_start] );
}

void Environment::finish()
{
    values_.push_back( NULL );
    buffer_.push_back( 0 );
}