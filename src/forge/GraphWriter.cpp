//
// GraphWriter.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "GraphWriter.hpp"
#include "Target.hpp"
#include <assert/assert.hpp>

using std::string;
using std::vector;
using namespace sweet::forge;

GraphWriter::GraphWriter( std::ostream* ostream )
: ostream_( ostream )
{
    SWEET_ASSERT( ostream_ );
}

void GraphWriter::write( Target* root_target )
{
    SWEET_ASSERT( root_target );
    const char FORMAT [] = "Sweet Build Graph";
    value( &FORMAT[0], sizeof(FORMAT) );
    const int VERSION = 32;
    value( VERSION );
    root_target->write( *this );
}

void GraphWriter::object_address( const void* address )
{
    ostream_->write( reinterpret_cast<const char*>(&address), sizeof(address) );
}

void GraphWriter::value( bool value )
{
    ostream_->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void GraphWriter::value( int value )
{
    ostream_->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void GraphWriter::value( uint64_t value )
{
    ostream_->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void GraphWriter::value( std::time_t value )
{
    ostream_->write( reinterpret_cast<const char*>(&value), sizeof(value) );
}

void GraphWriter::value( const std::string& value )
{
    size_t size = value.size();
    ostream_->write( reinterpret_cast<const char*>(&size), sizeof(size) );
    ostream_->write( reinterpret_cast<const char*>(value.c_str()), size );
}

void GraphWriter::value( const char* value, size_t size )
{
    ostream_->write( value, size );
}

void GraphWriter::value( const std::filesystem::file_time_type& time )
{
    value( reinterpret_cast<const char*>(&time), sizeof(time) );
}

void GraphWriter::value( const std::vector<std::string>& values )
{
    size_t length = values.size();
    ostream_->write( reinterpret_cast<const char*>(&length), sizeof(length) );
    for ( vector<string>::const_iterator i = values.begin(); i != values.end(); ++i )
    {
        value( *i );
    }
}

void GraphWriter::value( const std::vector<Target*>& values )
{
    size_t length = values.size();
    ostream_->write( reinterpret_cast<const char*>(&length), sizeof(length) );
    for ( vector<Target*>::const_iterator i = values.begin(); i != values.end(); ++i )
    {
        Target* target = *i;
        SWEET_ASSERT( target );
        target->write( *this );
    }
}

void GraphWriter::refer( const std::vector<Target*>& values )
{
    size_t length = values.size();
    ostream_->write( reinterpret_cast<const char*>(&length), sizeof(length) );
    for ( vector<Target*>::const_iterator i = values.begin(); i != values.end(); ++i )
    {
        Target* target = *i;
        SWEET_ASSERT( target );
        ostream_->write( reinterpret_cast<const char*>(&target), sizeof(target) );
    }
}
