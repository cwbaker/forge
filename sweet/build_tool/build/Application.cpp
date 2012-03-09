//
// Application.cpp
// Copyright (c) 2007 - 2011 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Application.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/cmdline/Parser.hpp>
#include <sweet/assert/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <string>
#include <vector>
#include <iostream>

using namespace sweet::build_tool;

Application::Application( int argc, char** argv )
: BuildToolEventSink(),
  result_( EXIT_SUCCESS )
{
    bool help = false;
    bool version = false;
    std::string filename = "";
    std::string directory = boost::filesystem::initial_path<boost::filesystem::path>().string();
    int jobs = 0;
    int warning_level = 0;
    bool stack_trace_enabled = false;
    std::vector<std::string> assignments;

    cmdline::Parser command_line_parser;
    command_line_parser.add_options()
        ( "help", "h", "Print this message and exit", &help )
        ( "version", "v", "Print the version and exit", &version )
        ( "filename", "f", "Set the filename of script to load", &filename )
        ( "warn", "W", "Set the warning level", &warning_level )
        ( "stack-trace", "s", "Enable stack traces in error messages", &stack_trace_enabled )
        ( &assignments )
    ;

    command_line_parser.parse( argc, argv );

    if ( version || help )
    {
        if ( version )
        {
            std::cout << "Sweet Build " << BUILD_VERSION << " \n";
            std::cout << "Copyright (c) 2007 - 2011 Charles Baker.  All rights reserved. \n";
        }

        if ( help )
        {
            std::cout << "Usage: build [options] [variable=value] ... \n";
            std::cout << "Options: \n";
            command_line_parser.print( stdout );
        }
    }
    else
    {
        BuildTool build_tool( directory, this );
        build_tool.set_warning_level( warning_level );
        build_tool.set_stack_trace_enabled( stack_trace_enabled );
        build_tool.assign( assignments );
        build_tool.search_up_for_root_directory( directory );
        build_tool.execute( filename );
    }
}

int Application::get_result() const
{
    return result_;
}

void Application::build_tool_output( BuildTool* build_tool, const char* message )
{
    SWEET_ASSERT( message );

    fputs( message, stdout );
    fputs( "\n", stdout );
    fflush( stdout );
}

void Application::build_tool_warning( BuildTool* build_tool, const char* message )
{
    SWEET_ASSERT( message );
    
    fputs( "build: ", stderr );
    fputs( message, stderr );
    fputs( ".\n", stderr );
    fflush( stderr );
}

void Application::build_tool_error( BuildTool* build_tool, const char* message )
{
    SWEET_ASSERT( message );
    
    fputs( "build: ", stderr );
    fputs( message, stderr );
    fputs( ".\n", stderr );
    fflush( stderr );
}
