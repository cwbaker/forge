//
// Application.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "stdafx.hpp"
#include "Application.hpp"
#include <forge/Forge.hpp>
#include <forge/path_functions.hpp>
#include <cmdline/Parser.hpp>
#include <error/ErrorPolicy.hpp>
#include <assert/assert.hpp>
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
using namespace sweet::forge;

Application::Application( int argc, char** argv )
: ForgeEventSink(),
  result_( EXIT_SUCCESS )
{
#ifdef BUILD_OS_WINDOWS
    _setmode( _fileno(stdout), _O_BINARY );
    _setmode( _fileno(stderr), _O_BINARY );
#endif

    bool help = false;
    bool version = false;
    std::string directory = boost::filesystem::initial_path<boost::filesystem::path>().generic_string();
    std::string root_directory;
    std::string filename = "forge.lua";
    bool stack_trace_enabled = false;    
    std::vector<std::string> assignments_and_commands;

    error::ErrorPolicy error_policy;
    cmdline::Parser command_line_parser;
    command_line_parser.add_options()
        ( "help", "h", "Print this message and exit", &help )
        ( "version", "v", "Print the version and exit", &version )
        ( "root", "r", "Set the root directory", &root_directory )
        ( "file", "f", "Set the name of the root build script", &filename )
        ( "stack-trace", "s", "Enable stack traces in error messages", &stack_trace_enabled )
        ( &assignments_and_commands )
    ;
    command_line_parser.parse( argc, argv );

    vector<string> assignments;
    vector<string> commands;

    if ( version )
    {
        std::cout << "Forge " << BUILD_VERSION << " \n";
        std::cout << "Copyright (c) 2007 - 2018 Charles Baker.  All rights reserved. \n";
    }

    if ( help )
    {
        std::cout << "Usage: forge [options] [variable=value] [command] ... \n";
        std::cout << "Options: \n";
        command_line_parser.print( stdout );
        commands.push_back( "help" );
    }

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
    
    if ( commands.empty() && !version )
    {
        const char* DEFAULT_COMMAND = "default";
        commands.push_back( DEFAULT_COMMAND );
    }

    if ( root_directory.empty() )
    {
        root_directory = forge::search_up_for_root_directory( directory, filename ).generic_string();
        error_policy.error( root_directory.empty(), "The file '%s' could not be found to identify the root directory", filename.c_str() );
    }

    vector<string>::const_iterator command = commands.begin(); 
    while ( error_policy.errors() == 0 && command != commands.end() )
    {
        Forge forge( directory, error_policy, this );
        forge.set_stack_trace_enabled( stack_trace_enabled );
        forge.set_root_directory( root_directory );
        forge.assign_global_variables( assignments );
        forge.execute( filename, *command );
        ++command;
    }
}

int Application::get_result() const
{
    return result_;
}

void Application::forge_output( Forge* /*forge*/, const char* message )
{
    SWEET_ASSERT( message );

    fputs( message, stdout );
    fputs( "\n", stdout );
    fflush( stdout );
}

void Application::forge_warning( Forge* /*forge*/, const char* message )
{
    SWEET_ASSERT( message );
    
    fputs( "forge: ", stderr );
    fputs( message, stderr );
    fputs( ".\n", stderr );
    fflush( stderr );
}

void Application::forge_error( Forge* /*forge*/, const char* message )
{
    SWEET_ASSERT( message );
    
    fputs( "forge: ", stderr );
    fputs( message, stderr );
    fputs( ".\n", stderr );
    fflush( stderr );

    result_ = EXIT_FAILURE;
}
