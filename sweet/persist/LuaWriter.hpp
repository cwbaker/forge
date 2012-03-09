//
// LuaWriter.hpp
// Copyright (c) 2008 - 2011 Charles Baker.  All rights reserved.
//

#ifndef SWEET_PERSIST_LUAWRITER_HPP_INCLUDED
#define SWEET_PERSIST_LUAWRITER_HPP_INCLUDED

#include "declspec.hpp"
#include "TextWriter.hpp"
#include <iostream>

namespace sweet
{

namespace persist
{

class Element;

/**
// Writes LUA archives.
*/
class SWEET_PERSIST_DECLSPEC LuaWriter : public TextWriter
{
    public:
        LuaWriter();
        LuaWriter( const TextWriter& writer );

        template <class Char, class Type> void write( const Char* filename, const char* name, Type& value );
        template <class Char, class Type> void write( const Char* filename, const char* name, const char* child_name, Type& container );
        template <class Char, class Type, size_t LENGTH> void write( const Char* filename, const char* name, const char* child_name, Type (& values)[LENGTH] );

        template <class Type> void write( std::ostream& stream, const char* name, Type& object );
        template <class Type> void write( std::ostream& stream, const char* name, const char* child_name, Type& container );
        template <class Type, size_t LENGTH> void write( std::ostream& stream, const char* name, const char* child_name, Type (& values)[LENGTH] );

    private:
        void write( const wchar_t* filename, const Element* element );
        void write( std::ostream& ostream, const Element* element );
        void write_element( std::ostream& stream, const Element* element, bool preserve_empty_elements, int depth );
        void write_indent( std::ostream& stream, int indent );
};

}

}


#endif
