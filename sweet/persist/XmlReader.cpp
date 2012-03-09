//
// XmlReader.cpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "XmlReader.hpp"
#include "XmlParser.hpp"
#include "Reader.ipp"
#include "Writer.ipp"

using namespace sweet::persist;

XmlReader::XmlReader()
: TextReader()
{
}

XmlReader::XmlReader( const TextReader& reader )
: TextReader( reader )
{
}

void XmlReader::parse( const char* filename, Element* element )
{
    XmlParser parser( filename, element );
}

void XmlReader::parse( const wchar_t* filename, Element* element )
{
    XmlParser parser( filename, element );
}

void XmlReader::parse( std::istream& stream, Element* element )
{
    XmlParser parser( stream, element );
}
