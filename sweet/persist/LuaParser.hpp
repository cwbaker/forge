//
// LuaParser.hpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_LUAPARSER_HPP_INCLUDED
#define SWEET_PERSIST_LUAPARSER_HPP_INCLUDED

#include <istream>

namespace sweet
{

namespace persist
{

class Element;

/**
// @internal
//
// Lua parser.
*/
class LuaParser
{
    public:
        LuaParser( const char* filename, Element* element );
        LuaParser( const wchar_t* filename, Element* element );
        LuaParser( std::istream& stream, Element* element );
    
    private:    
        void parse( const char* filename, std::istream& stream, Element* element );
};

}

}

#endif
