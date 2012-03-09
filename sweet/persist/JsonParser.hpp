//
// JsonParser.hpp
// Copyright (c) 2009 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_JSONPARSER_HPP_INCLUDED
#define SWEET_PERSIST_JSONPARSER_HPP_INCLUDED

#include <istream>

namespace sweet
{

namespace persist
{

class Element;

/**
// @internal
//
// JSON parser.
*/
class JsonParser
{
    public:
        JsonParser( const char* filename, Element* element );
        JsonParser( const wchar_t* filename, Element* element );
        JsonParser( std::istream& stream, Element* element );
    
    private:    
        void parse( const char* filename, std::istream& stream, Element* element );
};

}

}

#endif
