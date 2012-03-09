//
// XmlParser.hpp
// Copyright (c) 2006 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_XMLPARSER_HPP_INCLUDED
#define SWEET_PERSIST_XMLPARSER_HPP_INCLUDED

#include <iostream>

namespace sweet
{

namespace persist
{

class Element;

/**
// @internal
//
// XML parser.
*/
class XmlParser
{
    public:
        XmlParser( const char* filename, Element* element );
        XmlParser( const wchar_t* filename, Element* element );
        XmlParser( std::istream& stream, Element* element );
    
    private:    
        void parse( const char* filename, std::istream& stream, Element* element );
};

}

}

#endif
