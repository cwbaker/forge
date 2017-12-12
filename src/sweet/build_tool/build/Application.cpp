//
// Application.cpp
// Copyright (c) 2007 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Application.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/cmdline/Parser.hpp>
#include <sweet/error/ErrorPolicy.hpp>
#include <sweet/assert/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#ifdef BUILD_OS_WINDOWS
#include <io.h>
#include <fcntl.h>
#endif

using std::string;
using std::vector;
using namespace sweet::build_tool;

Application::Application( int argc, char** argv )
: BuildToolEventSink(),
  result_( EXIT_SUCCESS )
{
#ifdef BUILD_OS_WINDOWS
    _setmode( _fileno(stdout), _O_BINARY );
    _setmode( _fileno(stderr), _O_BINARY );
#endif

    bool help = false;
    bool version = false;
    std::string filename = "";
    std::string directory = boost::filesystem::initial_path<boost::filesystem::path>().generic_string();
    int warning_level = 0;
    bool stack_trace_enabled = false;
    std::vector<std::string> assignments_and_commands;

    cmdline::Parser command_line_parser;
    command_line_parser.add_options()
        ( "help", "h", "Print this message and exit", &help )
        ( "version", "v", "Print the version and exit", &version )
        ( "file", "f", "Set the script file to load", &filename )
        ( "warn", "W", "Set the warning level", &warning_level )
        ( "stack-trace", "s", "Enable stack traces in error messages", &stack_trace_enabled )
        ( &assignments_and_commands )
    ;
    command_line_parser.parse( argc, argv );

    if ( version || help )
    {
        if ( version )
        {
            std::cout << "Sweet Build " << BUILD_VERSION << " \n";
            std::cout << "Copyright (c) 2007 - 2016 Charles Baker.  All rights reserved. \n";
        }

        if ( help )
        {
            std::cout << "Usage: build [options] [variable=value] [command] ... \n";
            std::cout << "Options: \n";
            command_line_parser.print( stdout );
        }
    }
    else
    {
        vector<string> assignments;
        vector<string> commands;
        for ( std::vector<std::string>::const_iterator i = assignments_and_commands.begin(); i != assignments_and_commands.end(); ++i )
        {
            std::string::size_type position = i->find( "=" );
            if ( position == std::string::npos )
            {
                commands.push_back( *i );
            }
            else
            {
                assignments.push_back( *i );
            }
        }
        if ( commands.empty() )
        {
            const char* DEFAULT_COMMAND = "default";
            commands.push_back( DEFAULT_COMMAND );
        }
    
        error::ErrorPolicy error_policy;
        BuildTool build_tool( directory, error_policy, this );
        build_tool.set_warning_level( warning_level );
        build_tool.set_stack_trace_enabled( stack_trace_enabled );
        build_tool.search_up_for_root_directory( directory );
        build_tool.assign( assignments );
        build_tool.execute( filename, commands );
    }
}

int Application::get_result() const
{
    return result_;
}

void Application::build_tool_output( BuildTool* /*build_tool*/, const char* message )
{
    SWEET_ASSERT( message );

    fputs( message, stdout );
    fputs( "\n", stdout );
    fflush( stdout );
}

void Application::build_tool_warning( BuildTool* /*build_tool*/, const char* message )
{
    SWEET_ASSERT( message );
    
    fputs( "build: ", stderr );
    fputs( message, stderr );
    fputs( ".\n", stderr );
    fflush( stderr );
}

void Application::build_tool_error( BuildTool* /*build_tool*/, const char* message )
{
    SWEET_ASSERT( message );
    
    fputs( "build: ", stderr );
    fputs( message, stderr );
    fputs( ".\n", stderr );
    fflush( stderr );

    result_ = EXIT_FAILURE;
}
