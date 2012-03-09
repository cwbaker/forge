//
// XmlWriter.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_XMLWRITER_IPP_INCLUDED
#define SWEET_PERSIST_XMLWRITER_IPP_INCLUDED

#include "XmlWriter.hpp"
#include <sweet/assert/assert.hpp>
#include "TextWriter.ipp"

namespace sweet
{

namespace persist
{

template <class Char, class Type> 
void XmlWriter::write( const Char* filename, const char* name, Type& object )
{
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );
    TextWriter::write( name, object );
    write( widen(filename).c_str(), get_element() );
}

template <class Char, class Type> 
void XmlWriter::write( const Char* filename, const char* name, const char* child_name, Type& container )
{
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );
    TextWriter::write( name, child_name, container );
    write( widen(filename).c_str(), get_element() );
}

template <class Char, class Type, size_t LENGTH> 
void XmlWriter::write( const Char* filename, const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );
    TextWriter::write( name, child_name, values );
    write( widen(filename).c_str(), get_element() );
}

template <class Type> 
void XmlWriter::write( std::ostream& stream, const char* name, Type& object )
{
    TextWriter::write( name, object );
    write( stream, get_element() );
}

template <class Type> 
void XmlWriter::write( std::ostream& stream, const char* name, const char* child_name, Type& container )
{
    TextWriter::write( name, child_name, container );
    write( stream, get_element() );
}

template <class Type, size_t LENGTH> 
void XmlWriter::write( std::ostream& stream, const char* name, const char* child_name, Type (& values)[LENGTH] )
{
    TextWriter::write( name, child_name, values );
    write( stream, get_element() );
}

}

}

#endif