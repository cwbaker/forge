//
// XmlWriter.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "XmlWriter.hpp"
#include "functions.hpp"
#include "Writer.ipp"
#include <fstream>

using namespace sweet::persist;

XmlWriter::XmlWriter()
: TextWriter()
{
}

XmlWriter::XmlWriter( const TextWriter& writer )
: TextWriter( writer )
{
}

void XmlWriter::write( const wchar_t* filename, const Element* element )
{
    SWEET_ASSERT( filename );
    SWEET_ASSERT( element );
    std::ofstream stream( narrow(std::wstring(filename)).c_str() );
    write( stream, element );
    stream.close();
}

void XmlWriter::write( std::ostream& stream, const Element* element )
{
    SWEET_ASSERT( element );
    stream.exceptions( std::wofstream::badbit | std::wofstream::failbit );
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    write_element( stream, element, element->is_flag(PERSIST_PRESERVE_EMPTY_ELEMENTS), 0 );
}

void XmlWriter::write_element( std::ostream& stream, const Element* element, bool preserve_empty_elements, int depth )
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
        return;
    }

    write_indent( stream, depth );
    stream << "<" << element->name().c_str();

    std::list<Attribute>::const_iterator attribute = element->attributes().begin();
    while ( attribute != element->attributes().end() )
    {        
        std::string value;
        attribute->string( &value );
        stream << " " << attribute->name() << "=\"";

        std::string::const_iterator character = value.begin();
        while ( character != value.end() )
        {
            switch ( *character )
            {
                case '&':
                    stream << "&amp;";
                    break;

                case '<':
                    stream << "&lt;";
                    break;

                case '>':
                    stream << "&gt;";
                    break;

                case '"':
                    stream << "&quot;";
                    break;

                default:
                    stream << *character;
                    break;
            }

            ++character;
        }

        stream << "\"";

        ++attribute;
    }

    if ( element->is_leaf() )
    {
        if ( element->attributes().empty() )
        {
            stream << " />\n";
        }
        else
        {
            stream << "/>\n";
        }
    }
    else
    {
        stream << ">\n";

        std::list<Element>::const_iterator child = element->elements().begin();
        while ( child != element->elements().end() )
        {
            write_element( stream, &(*child), element->is_flag(PERSIST_PRESERVE_EMPTY_ELEMENTS), depth + 1 );
            ++child;
        }

        write_indent( stream, depth );
        stream << "</" << element->name() << ">\n";
    }
}

void XmlWriter::write_indent( std::ostream& stream, int indent )
{
    for ( int i = 0; i < indent; ++i )
    {
        stream << "    ";
    }    
}
