//
// GraphReader.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "GraphReader.hpp"
#include "Target.hpp"
#include <sweet/error/ErrorPolicy.hpp>
#include <sweet/assert/assert.hpp>
#include <memory>

using std::map;
using std::vector;
using std::string;
using std::unique_ptr;
using std::make_pair;
using namespace sweet;
using namespace sweet::build_tool;

GraphReader::GraphReader( std::istream* istream, error::ErrorPolicy* error_policy  )
: istream_( istream ),
  error_policy_( error_policy ),
  address_by_old_address_(),
  version_( 0 )
{
    SWEET_ASSERT( istream_ );
    SWEET_ASSERT( error_policy_ );
}

void* GraphReader::find_address_by_old_address( const void* old_address ) const
{
    map<const void*, void*>::const_iterator i = address_by_old_address_.find( old_address );
    return i != address_by_old_address_.end() ? i->second : nullptr;
}

std::unique_ptr<Target> GraphReader::read( const std::string& filename )
{
    const char FORMAT [] = "Sweet Build Graph";
    char format [sizeof(FORMAT)];
    value( &format[0], sizeof(format) );
    if ( strncmp(format, FORMAT, sizeof(FORMAT)) != 0 )
    {
        error_policy_->print( "The file '%s' is not a valid dependency graph", filename.c_str() );
        return unique_ptr<Target>();
    }

    unique_ptr<Target> root_target;
    value( &version_ );
    root_target.reset( new Target );
    root_target->read( *this );
    root_target->resolve( *this );
    return root_target;
}

void GraphReader::object_address( void* address )
{
    const void* old_address = nullptr;
    istream_->read( reinterpret_cast<char*>(&old_address), sizeof(old_address) );
    address_by_old_address_.insert( make_pair(old_address, address) );
}

void GraphReader::value( bool* value )
{
    istream_->read( reinterpret_cast<char*>(value), sizeof(*value) );
}

void GraphReader::value( int* value )
{
    istream_->read( reinterpret_cast<char*>(value), sizeof(*value) );
}

void GraphReader::value( std::time_t* value )
{
    istream_->read( reinterpret_cast<char*>(value), sizeof(*value) );
}

void GraphReader::value( std::string* value )
{
    size_t size = 0;
    istream_->read( reinterpret_cast<char*>(&size), sizeof(size) );
    value->resize( size );
    istream_->read( reinterpret_cast<char*>(&(*value)[0]), size );
}

void GraphReader::value( char* value, size_t size )
{
    istream_->read( value, size );
}

void GraphReader::value( std::vector<std::string>* values )
{
    size_t length = 0;
    istream_->read( reinterpret_cast<char*>(&length), sizeof(length) );
    values->resize( length );
    for ( vector<string>::iterator i = values->begin(); i != values->end(); ++i )
    {
        value( &(*i) );
    }
}

void GraphReader::value( std::vector<Target*>* values )
{
    size_t length = 0;
    istream_->read( reinterpret_cast<char*>(&length), sizeof(length) );
    values->resize( length );
    for ( vector<Target*>::iterator i = values->begin(); i != values->end(); ++i )
    {
        unique_ptr<Target> target( new Target );
        target->read( *this );
        *i = target.release();
    }
}

void GraphReader::refer( std::vector<Target*>* values )
{
    size_t length = 0;
    istream_->read( reinterpret_cast<char*>(&length), sizeof(length) );
    values->resize( length );
    for ( vector<Target*>::iterator i = values->begin(); i != values->end(); ++i )
    {
        Target* target = nullptr;
        istream_->read( reinterpret_cast<char*>(&target), sizeof(target) );
        *i = target;
    }
}
