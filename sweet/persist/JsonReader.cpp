//
// JsonReader.cpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "JsonReader.hpp"
#include "JsonParser.hpp"
#include "Reader.ipp"
#include "Writer.ipp"

using namespace sweet::persist;

/**
// Constructor.
*/
JsonReader::JsonReader()
: TextReader()
{
}


/**
// Constructor.
//
// @param reader
//  The TextReader to copy.
*/
JsonReader::JsonReader( const TextReader& reader )
: TextReader( reader )
{
}


/**
// Parse the JSON file \e filename into the element and attribute tree rooted
// at \e element.
//
// @param filename
//  The path to the JSON file to read.
//
// @param element
//  The root element of the tree to read into.
*/
void JsonReader::parse( const char* filename, Element* element )
{
    JsonParser parser( filename, element );
}


/**
// Parse the JSON file \e filename into the element and attribute tree rooted
// at \e element.
//
// @param filename
//  The path to the JSON file to read.
//
// @param element
//  The root element of the tree to read into.
*/
void JsonReader::parse( const wchar_t* filename, Element* element )
{
    JsonParser parser( filename, element );
}


/**
// Parse a stream containing JSON into the element and attribute tree rooted
// at \e element.
//
// @param stream
//  The stream to parse the JSON from.
//
// @param element
//  The root element of the tree to read into.
*/
void JsonReader::parse( std::istream& stream, Element* element )
{
    JsonParser parser( stream, element );
}
