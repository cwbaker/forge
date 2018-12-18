//
// Environment.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Environment.hpp"
#include <assert/assert.hpp>

using std::vector;
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

    uintptr_t key_start = buffer_.size();
    buffer_.resize( buffer_.size() + key_length + value_length + 2 );
    strncpy( &buffer_[key_start], key, key_length + 1 );
    buffer_[key_start + key_length] = '=';
    size_t value_start = key_start + key_length + 1;
    strncpy( &buffer_[value_start], value, value_length + 1 );
    buffer_[value_start + value_length] = 0;

    values_.push_back( (char*) key_start );
}

void Environment::prepare()
{
    for ( vector<char*>::iterator value = values_.begin(); value != values_.end(); ++value )
    {
        *value = buffer_.data() + (uintptr_t) *value;
    }
    values_.push_back( NULL );
    buffer_.push_back( 0 );
}
