//
// XmlReader.ipp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_XMLREADER_IPP_INCLUDED
#define SWEET_PERSIST_XMLREADER_IPP_INCLUDED

#include "XmlReader.hpp"
#include "TextReader.ipp"
#include "functions.hpp"

namespace sweet
{

namespace persist
{

/**
// Read an XML archive into an object.
//
// @param filename
//  The name of the XML archive to read.
//
// @param name
//  The name of the top level element.
//
// @param object
//  The object to read the archive into.
//
// @return
//  Nothing.
*/
template <class Char, class Type> 
void XmlReader::read( const Char* filename, const char* name, Type& object )
{
//
// Set the directory that this archive is being written to in the base
// Reader class so that it can be retrieved later for path filtering.
//
    SWEET_ASSERT( filename != 0 );
    set_filename( widen(filename) );

//
// Clear out the Element and Attribute tree.
//
    SWEET_ASSERT( get_element() != 0 );
    get_element()->clear();

//
// Parse the XML file and then process the tree of data that has been 
// read in into the C++ model.
//
    parse( filename, get_element() );
    TextReader::read( name, object );
}

/**
// Read an XML archive into a container of objects.
//
// @param filename
//  The name of the XML archive to read.
//
// @param name
//  The name of the top level element.
//
// @param child_name
//  The name of each child element.
//
// @param container
//  The container of objects to read the archive into.
*/
template <class Char, class Type> 
void 
XmlReader::read( const Char* filename, const char* name, const char* child_name, Type& container )
{
//
// Set the directory that this archive is being written to in the base
// Reader class so that it can be retrieved later for path filtering.
//
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );

//
// Clear out the Element and Attribute tree.
//
    SWEET_ASSERT( get_element() != NULL );
    get_element()->clear();

//
// Parse the XML file and then process the tree of data that has been 
// read in into the C++ model.
//
    parse( filename, get_element() );
    TextReader::read( name, child_name, container );
}

/**
// Read an XML archive into an array of objects.
//
// @param filename
//  The name of the XML archive to read.
//
// @param name
//  The name of the top level element.
//
// @param child_name
//  The name of each child element.
//
// @param values
//  The array to read the archive into.
*/
template <class Char, class Type, size_t LENGTH> 
void 
XmlReader::read( const Char* filename, const char* name, const char* child_name, Type (& values)[LENGTH] )
{
//
// Set the directory that this archive is being written to in the base
// Reader class so that it can be retrieved later for path filtering.
//
    SWEET_ASSERT( filename );
    set_filename( widen(filename) );

//
// Clear out the Element and Attribute tree.
//
    SWEET_ASSERT( get_element() != NULL );
    get_element()->clear();

//
// Parse the XML file and then process the tree of data that has been read 
// in into the C++ model.
//
    parse( filename, get_element() );
    TextReader::read( name, child_name, values );
}

/**
// Read an XML archive into an object.
//
// @param stream
//  The stream to read the XML archive from.
//
// @param name
//  The name of the top level element.
//
// @param object
//  The object to read the archive into.
*/
template <class Type> 
void 
XmlReader::read( std::istream& stream, const char* name, Type& object )
{
//
// Clear out the Element and Attribute tree.
//
    SWEET_ASSERT( get_element() != NULL );
    get_element()->clear();

//
// Parse the XML file and then process the tree of data that has been read 
// in into the C++ model.
//
    parse( stream, get_element() );
    TextReader::read( name, object );
}

/**
// Read an XML archive into a container.
//
// @param stream
//  The stream to read the XML archive from.
//
// @param name
//  The name of the top level element.
//
// @param child_name
//  The name of each child element.
//
// @param container
//  The container of objects to read the archive into.
*/
template <class Type> 
void 
XmlReader::read( std::istream& stream, const char* name, const char* child_name, Type& container )
{
//
// Clear out the Element and Attribute tree.
//
    SWEET_ASSERT( get_element() != NULL );
    get_element()->clear();

//
// Parse the XML file and then process the tree of data that has been read 
// in into the C++ model.
//
    parse( stream, get_element() );
    TextReader::read( name, child_name, container );
}

/**
// Read an XML archive into an array.
//
// @param stream
//  The stream to read the XML archive from.
//
// @param name
//  The name of the top level element.
//
// @param child_name
//  The name of each child element.
//
// @param values
//  The array to read the archive into.
*/
template <class Type, size_t LENGTH> 
void 
XmlReader::read( std::istream& stream, const char* name, const char* child_name, Type (& values)[LENGTH] )
{
//
// Clear out the Element and Attribute tree.
//
    SWEET_ASSERT( get_element() != NULL );
    get_element()->clear();

//
// Parse the XML file and then process the tree of data that has been read 
// in into the C++ model.
//
    parse( stream, get_element() );
    TextReader::read( name, child_name, values );
}

}

}

#endif

