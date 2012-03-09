//
// LuaReader.cpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "LuaReader.hpp"
#include "LuaParser.hpp"
#include "Reader.ipp"
#include "Writer.ipp"

using namespace sweet::persist;

LuaReader::LuaReader()
: TextReader()
{
}

LuaReader::LuaReader( const TextReader& reader )
: TextReader( reader )
{
}

void LuaReader::parse( const char* filename, Element* element )
{
    LuaParser parser( filename, element );
}

void LuaReader::parse( const wchar_t* filename, Element* element )
{
    LuaParser parser( filename, element );
}

void LuaReader::parse( std::istream& stream, Element* element )
{
    LuaParser parser( stream, element );
}
