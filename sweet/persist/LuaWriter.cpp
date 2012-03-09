//
// LuaWriter.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "LuaWriter.hpp"
#include "Writer.ipp"
#include "functions.hpp"
#include <fstream>

using namespace sweet::persist;

LuaWriter::LuaWriter()
: TextWriter()
{
}

LuaWriter::LuaWriter( const TextWriter& writer )
: TextWriter( writer )
{
}

void LuaWriter::write( const wchar_t* filename, const Element* element )
{
    std::ofstream stream( narrow(std::wstring(filename)).c_str() );
    write( stream, element );
    stream.close();
}

void LuaWriter::write( std::ostream& stream, const Element* element )
{
    stream.exceptions( std::wofstream::badbit | std::wofstream::failbit );
    write_element( stream, element, element->is_flag(PERSIST_PRESERVE_EMPTY_ELEMENTS), 0 );
}

void LuaWriter::write_element( std::ostream& stream, const Element* element, bool preserve_empty_elements, int depth )
{
    if ( element == NULL )
    {
    //
    // @todo
    //  Decide whether it is valid to write out empty Element and Attribute
    //  trees or whether an exception should be thrown here.
    //
        SWEET_ASSERT( false );
        return;
    }

    if ( depth > 0 && element->elements().empty() && element->attributes().empty() && !preserve_empty_elements )
    {
        write_indent( stream, depth );
        stream << element->name() << " = nil";
        return;
    }

    write_indent( stream, depth );
    stream << element->name() << " = {\n";

    std::list<Attribute>::const_iterator attribute = element->attributes().begin();
    while ( attribute != element->attributes().end() )
    {        
        write_indent( stream, depth + 1 );
        stream << attribute->name() << " = ";

        std::string value;
        switch ( attribute->type() )
        {
            case TYPE_VOID:
                stream << "nil";
                break;

            case TYPE_BOOLEAN:
                stream << attribute->boolean() ? "true" : "false";
                break;

            case TYPE_INTEGER:
            case TYPE_UNSIGNED_INTEGER:
            case TYPE_REAL:
            case TYPE_NUMBER:
            {
                attribute->string( &value );
                stream << value;
                break;
            }

            case TYPE_ADDRESS:
            case TYPE_STRING:
            {
                stream << "\"";
                attribute->string( &value );

                std::string::const_iterator character = value.begin();
                while ( character != value.end() )
                {
                    switch ( *character )
                    {
                        case '"':
                            stream << "\\\"";
                            break;

                        case '\\':
                            stream << "\\\\";
                            break;

                        case '\b':
                            stream << "\\b";
                            break;

                        case '\f':
                            stream << "\\f";
                            break;

                        case '\n':
                            stream << "\\n";
                            break;

                        case '\r':
                            stream << "\\r";
                            break;

                        case '\t':
                            stream << "\\t";
                            break;

                        case '\v':
                            stream << "\\v";
                            break;

                        default:
                            stream << *character;
                            break;
                    }

                    ++character;
                }

                stream << "\"";
                break;
            }

            default:
                SWEET_ASSERT( false );
                stream << "nil";
                break;
        }

        ++attribute;
        if ( attribute != element->attributes().end() || !element->elements().empty() )
        {
            stream << ",\n";    
        }
        else
        {
            stream << "\n";
        }
    }

    std::list<Element>::const_iterator child = element->elements().begin();
    while ( child != element->elements().end() )
    {
        write_element( stream, &(*child), element->is_flag(PERSIST_PRESERVE_EMPTY_ELEMENTS), depth + 1 );

        ++child;
        if ( child != element->elements().end() )
        {
            stream << ",\n";
        }
        else
        {
            stream << "\n";
        }
    }

    write_indent( stream, depth );
    stream << "}";
}

void LuaWriter::write_indent( std::ostream& stream, int indent )
{
    for ( int i = 0; i < indent; ++i )
    {
        stream << "    ";
    }    
}
