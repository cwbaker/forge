//
// XmlParser.cpp
// Copyright (c) 2006 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "XmlParser.hpp"
#include "parser.hpp"
#include "xml_parser.hpp"
#include "Element.hpp"
#include "Error.hpp"
#include "functions.hpp"
#include <fstream>

using std::vector;
using std::istream_iterator;
using namespace sweet::parser;
using namespace sweet::persist;

struct XmlParserEventSink : public ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>
{    
    const char* filename_;
    std::list<Element*> elements_;
    
    XmlParserEventSink( const char* filename, Element* element )
    : filename_( filename ),
      elements_()
    {
        SWEET_ASSERT( filename_ );
        SWEET_ASSERT( element );
        elements_.push_back( element );
    }
    
    void parser_vprintf( const char* format, va_list args )
    {
        vprintf( format, args );
    }
    
    void parser_error( const ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>* parser, const char* message )    
    {
    }

    void begin_element( const std::string& name )
    {
        SWEET_ASSERT( !name.empty() );
        SWEET_ASSERT( !elements_.empty() );
        SWEET_ASSERT( elements_.back() );
        
        Element* element = elements_.back()->add_element( Element(name, NULL) );
        SWEET_ASSERT( element );
        elements_.push_back( element );
    }
    
    void end_element()
    {
        SWEET_ASSERT( !elements_.empty() );    
        elements_.pop_back();
    }   

    void attribute( const std::string& name, const std::string& value )
    {
        SWEET_ASSERT( !name.empty() );
        SWEET_ASSERT( !elements_.empty() );
        SWEET_ASSERT( elements_.back() );
        elements_.back()->add_attribute( Attribute(name, value) );
    }
    
    void parser_error( const Parser<PositionIterator<istream_iterator<unsigned char> >, void*, char>* parser, const char* message )
    {
        SWEET_ERROR( ParsingFileFailedError("%s(%d) : %s", filename_, parser->position().line(), message) );
    }
};

static void string_( PositionIterator<istream_iterator<unsigned char> >* begin, PositionIterator<istream_iterator<unsigned char> > end, std::string* lexeme, int* symbol )
{
    SWEET_ASSERT( begin );
    SWEET_ASSERT( lexeme );
    SWEET_ASSERT( lexeme->length() == 1 );

    PositionIterator<istream_iterator<unsigned char> > position = *begin;
    int terminator = lexeme->at( 0 );
    SWEET_ASSERT( terminator == '\'' || terminator == '"' );
    lexeme->clear();
    
    while ( position != end && *position != terminator )
    {
        if ( *position != '&' )
        {
            *lexeme += *position;
            ++position;
        }
        else
        {
            ++position;            
            std::string reference;
            while ( position != end && *position != terminator && *position != ';' )
            {
                reference += *position;
                ++position;
            }
            
            if ( position != end && *position != terminator )
            {
                SWEET_ASSERT( *position == ';' );
                ++position;
            }

            struct ReferenceMap
            {
                const char* reference;
                const char* replacement;
            };

            static const ReferenceMap REFERENCE_MAP[] =
            {
                { "amp",  "&"  },
                { "lt",   "<"  },
                { "gt",   ">"  },
                { "quot", "\"" },
                { NULL, NULL }
            };

            const ReferenceMap* map = REFERENCE_MAP;
            while ( map->reference != 0 && reference != map->reference )
            {    
                ++map;
            }

            if ( map->replacement != NULL )
            {
                lexeme->append( map->replacement );
            }
        }
    }
    
    if ( position != end )
    {
        ++position;
    }
    
    *begin = position;
}

static void* begin_element( int symbol, const ParserNode<void*, char>* start, const ParserNode<void*, char>* finish, ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>* context )
{
    XmlParserEventSink* xml_parser_context = static_cast<XmlParserEventSink*>( context );
    SWEET_ASSERT( xml_parser_context );
    xml_parser_context->begin_element( start[1].lexeme_ );
    return NULL;
}

static void* end_element( int symbol, const ParserNode<void*, char>* start, const ParserNode<void*, char>* finish, ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>* context )
{
    XmlParserEventSink* xml_parser_context = static_cast<XmlParserEventSink*>( context );
    SWEET_ASSERT( xml_parser_context );
    xml_parser_context->end_element();
    return NULL;
}

static void* attribute( int symbol, const ParserNode<void*, char>* start, const ParserNode<void*, char>* finish, ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>* context )
{
    XmlParserEventSink* xml_parser_context = static_cast<XmlParserEventSink*>( context );
    SWEET_ASSERT( xml_parser_context );
    xml_parser_context->attribute( start[0].lexeme_, start[2].lexeme_ );
    return NULL;
}

XmlParser::XmlParser( const char* filename, Element* element )
{
    SWEET_ASSERT( filename );

    std::ifstream stream( filename, std::ios::binary );
    if ( !stream.is_open() )
    {
        SWEET_ERROR( OpeningFileFailedError("Opening '%s' failed", filename) );
    }
    
    parse( filename, stream, element );
}

XmlParser::XmlParser( const wchar_t* filename, Element* element )
{
    std::ifstream stream( narrow(filename).c_str(), std::ios::binary );
    if ( !stream.is_open() )
    {
        SWEET_ERROR( OpeningFileFailedError("Opening '%s' failed", narrow(filename).c_str()) );
    }    
    parse( narrow(filename).c_str(), stream, element );
}

XmlParser::XmlParser( std::istream& stream, Element* element )
{
    parse( "", stream, element );
}

void XmlParser::parse( const char* filename, std::istream& stream, Element* element )
{
    stream.unsetf( std::iostream::skipws );
    stream.exceptions( std::iostream::badbit );
    
    XmlParserEventSink xml_parser_event_sink( filename, element );
    Parser<PositionIterator<istream_iterator<unsigned char> >, void*, char> parser( &xml_parser_state_machine, &xml_parser_event_sink );
    parser.lexer_action_handlers()
        ( "string", &string_ )
    ;
    parser.parser_action_handlers()
        ( "begin_element", &begin_element )
        ( "end_element", &end_element )
        ( "attribute", &attribute )
    ;
    
    parser.parse( PositionIterator<istream_iterator<unsigned char> >(istream_iterator<unsigned char>(stream), istream_iterator<unsigned char>()), PositionIterator<istream_iterator<unsigned char> >() );
    if ( !parser.accepted() || !parser.full() )
    {
        SWEET_ERROR( ReadingFileFailedError("Parsing a stream failed") );
    }
}
