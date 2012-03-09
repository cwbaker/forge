//
// JsonParser.cpp
// Copyright (c) 2009 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "JsonParser.hpp"
#include "parser.hpp"
#include "json_parser.hpp"
#include "Element.hpp"
#include "Error.hpp"
#include "functions.hpp"
#include <fstream>
#include <stdlib.h>

using std::vector;
using std::istream_iterator;
using namespace sweet::parser;
using namespace sweet::persist;

struct JsonParserEventSink : public ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>
{    
    const char* filename_;
    std::list<Element*> elements_;
    
    JsonParserEventSink( const char* filename, Element* element )
    : filename_( filename ),
      elements_()
    {
        SWEET_ASSERT( filename_ );
        SWEET_ASSERT( element );
        elements_.push_back( element );
    }

    void begin_element( const std::string& name )
    {
        SWEET_ASSERT( !name.empty() );
        SWEET_ASSERT( !elements_.empty() );
        SWEET_ASSERT( elements_.back() != NULL );
        
        Element* element = elements_.back()->add_element( Element(name, NULL) );
        SWEET_ASSERT( element != NULL );
        elements_.push_back( element );
    }
    
    void end_element()
    {
        SWEET_ASSERT( !elements_.empty() );        
        elements_.pop_back();
    }

    void attribute( const Attribute& attribute )
    {
        SWEET_ASSERT( !elements_.empty() );
        SWEET_ASSERT( elements_.back() != NULL );
        elements_.back()->add_attribute( attribute );
    }

    void parser_error( const Parser<PositionIterator<PositionIterator<istream_iterator<unsigned char> > >, void*, char>* parser, const char* message )
    {
        SWEET_ERROR( ParsingFileFailedError("%s(%d) : %s", filename_, parser->position().line(), message) );
    }    
};

static void string_( PositionIterator<istream_iterator<unsigned char> >* begin, PositionIterator<istream_iterator<unsigned char> > end, std::string* lexeme, int* symbol )
{
    SWEET_ASSERT( begin != NULL );
    SWEET_ASSERT( lexeme != NULL );
    SWEET_ASSERT( lexeme->length() == 1 );

    PositionIterator<istream_iterator<unsigned char> > position = *begin;
    int terminator = lexeme->at( 0 );
    SWEET_ASSERT( terminator == '\'' || terminator == '"' );
    lexeme->clear();
    
    while ( *position != terminator && position != end )
    {
        if ( *position != '\\' )
        {
            *lexeme += *position;
            ++position;
        }
        else
        {
            ++position;
            if ( position != end )
            {
                switch ( *position )
                {
                    case 'b':
                        *lexeme += '\b';
                        ++position;
                        break;
                    
                    case 'f':
                        *lexeme += '\f';
                        ++position;
                        break;
                    
                    case 'n':
                        *lexeme += '\n';
                        ++position;
                        break;
                    
                    case 'r':
                        *lexeme += '\r';
                        ++position;
                        break;
                    
                    case 't':
                        *lexeme += '\t';
                        ++position;
                        break;
                        
                    case 'x':
                    case 'X':
                    {
                        ++position;
                        int character = 0;
                        while ( position != end && isxdigit(*position) )
                        {
                            character <<= 4;                        
                            if ( *position >= '0' && *position <= '9' )
                            {
                                character += *position - '0';
                            }
                            else if ( *position >= 'a' && *position <= 'f' )
                            {
                                character += *position - 'a' + 10;
                            }
                            else if ( *position >= 'A' && *position <= 'F' )
                            {
                                character += *position - 'A' + 10;
                            }
                            
                            ++position;                            
                        }
                        
                        *lexeme += character;
                        break;
                    }
                        
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    {
                        int character = 0;
                        int count = 0;
                        while ( position != end && count < 3 && *position >= '0' && *position <= '7' )
                        {
                            character <<= 3;
                            character += *position - '0';
                            ++position;
                            ++count;
                        }
                        
                        *lexeme += character;                
                        break;
                    }
                        
                    default:
                        *lexeme += *position;
                        ++position;
                        break;
                }
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
    JsonParserEventSink* json_parser_context = static_cast<JsonParserEventSink*>( context );
    SWEET_ASSERT( json_parser_context );
    json_parser_context->begin_element( start[0].lexeme_ );
    return NULL;
}

static void* end_element( int symbol, const ParserNode<void*, char>* start, const ParserNode<void*, char>* finish, ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>* context )
{
    JsonParserEventSink* json_parser_context = static_cast<JsonParserEventSink*>( context );
    SWEET_ASSERT( json_parser_context != NULL );
    json_parser_context->end_element();
    return NULL;
}

static void* null_attribute( int symbol, const ParserNode<void*, char>* start, const ParserNode<void*, char>* finish, ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>* context )
{
    JsonParserEventSink* json_parser_context = static_cast<JsonParserEventSink*>( context );
    SWEET_ASSERT( json_parser_context );
    json_parser_context->attribute( Attribute(start[0].lexeme_) );
    return NULL;
}

static void* boolean_attribute( int symbol, const ParserNode<void*, char>* start, const ParserNode<void*, char>* finish, ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>* context )
{
    JsonParserEventSink* json_parser_context = static_cast<JsonParserEventSink*>( context );
    SWEET_ASSERT( json_parser_context );
    json_parser_context->attribute( Attribute(start[0].lexeme_, start[2].lexeme_ == "true") );
    return NULL;
}

static void* integer_attribute( int symbol, const ParserNode<void*, char>* start, const ParserNode<void*, char>* finish, ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>* context )
{
    JsonParserEventSink* json_parser_context = static_cast<JsonParserEventSink*>( context );
    SWEET_ASSERT( json_parser_context );
    json_parser_context->attribute( Attribute(start[0].lexeme_, atoi(start[2].lexeme_.c_str())) );
    return NULL;
}

static void* real_attribute( int symbol, const ParserNode<void*, char>* start, const ParserNode<void*, char>* finish, ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>* context )
{
    JsonParserEventSink* json_parser_context = static_cast<JsonParserEventSink*>( context );
    SWEET_ASSERT( json_parser_context );
    json_parser_context->attribute( Attribute(start[0].lexeme_, atof(start[2].lexeme_.c_str())) );
    return NULL;
}

static void* string_attribute( int symbol, const ParserNode<void*, char>* start, const ParserNode<void*, char>* finish, ParserEventSink<PositionIterator<istream_iterator<unsigned char> >, void*, char>* context )
{
    JsonParserEventSink* json_parser_context = static_cast<JsonParserEventSink*>( context );
    SWEET_ASSERT( json_parser_context );
    json_parser_context->attribute( Attribute(start[0].lexeme_, start[2].lexeme_) );
    return NULL;
}

JsonParser::JsonParser( const char* filename, Element* element )
{
    SWEET_ASSERT( filename );

    std::ifstream stream( filename, std::ios::binary );
    if ( !stream.is_open() )
    {
        SWEET_ERROR( OpeningFileFailedError("Opening '%s' failed", filename) );
    }

    parse( filename, stream, element );
}

JsonParser::JsonParser( const wchar_t* filename, Element* element )
{
    SWEET_ASSERT( filename );

    std::ifstream stream( narrow(filename).c_str(), std::ios::binary );
    if ( !stream.is_open() )
    {
        SWEET_ERROR( OpeningFileFailedError("Opening '%s' failed", narrow(filename).c_str()) );
    }

    parse( narrow(filename).c_str(), stream, element );
}

JsonParser::JsonParser( std::istream& stream, Element* element )
{
    parse( "", stream, element );
}

void JsonParser::parse( const char* filename, std::istream& stream, Element* element )
{
    stream.unsetf( std::iostream::skipws );
    stream.exceptions( std::iostream::badbit );
    
    JsonParserEventSink json_parser_event_sink( filename, element );
    Parser<PositionIterator<istream_iterator<unsigned char> >, void*, char> parser( &json_parser_state_machine, &json_parser_event_sink );
    parser.lexer_action_handlers()
        ( "string", &string_ )
    ;
    parser.parser_action_handlers()
        ( "begin_element", &begin_element )
        ( "end_element", &end_element )
        ( "null_attribute", &null_attribute )
        ( "boolean_attribute", &boolean_attribute )
        ( "integer_attribute", &integer_attribute )
        ( "real_attribute", &real_attribute )
        ( "string_attribute", &string_attribute )
    ;
    
    parser.parse( PositionIterator<istream_iterator<unsigned char> >(istream_iterator<unsigned char>(stream), istream_iterator<unsigned char>()), PositionIterator<istream_iterator<unsigned char> >() );
    if ( !parser.accepted() || !parser.full() )
    {
        SWEET_ERROR( ReadingFileFailedError("Parsing a stream failed") );
    }
}
