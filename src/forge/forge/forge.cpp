//
// main.cpp
// Copyright (c) Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "EventSink.hpp"
#include <forge/Forge.hpp>
#include <forge/path_functions.hpp>
#include <cmdline/Parser.hpp>
#include <error/ErrorPolicy.hpp>
#include <assert/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <stdio.h>
#include <stdlib.h>
#ifdef BUILD_OS_WINDOWS
#include <io.h>
#include <fcntl.h>
#endif

using std::string;
using std::vector;
using namespace boost::filesystem;
using namespace sweet;
using namespace sweet::forge;

int main( int argc, char** argv )
{
    try
    {
#ifdef BUILD_OS_WINDOWS
        _setmode( _fileno(stdout), _O_BINARY );
        _setmode( _fileno(stderr), _O_BINARY );
#endif

        bool help = false;
        bool version = false;
        string directory = initial_path<path>().generic_string();
        string root_directory;
        string build_script = "forge.lua";
        bool stack_trace_enabled = false;    
        vector<string> assignments_and_commands;

        EventSink event_sink;
        error::ErrorPolicy error_policy;
        cmdline::Parser command_line_parser;
        command_line_parser.add_options()
            ( "help", "h", "Print this message and exit", &help )
            ( "version", "v", "Print the version and exit", &version )
            ( "root", "r", "Set root directory", &root_directory )
            ( "build-script", "b", "Set build script filename", &build_script )
            ( "stack-trace", "s", "Stack traces on error", &stack_trace_enabled )
            ( &assignments_and_commands )
        ;
        command_line_parser.parse( argc, argv );

        if ( version )
        {
            std::cout << "Forge " << BUILD_VERSION << " \n";
            std::cout << "Copyright (c) 2007 - 2023 Charles Baker.  All rights reserved. \n";
        }

        if ( help )
        {
            std::cout << "Usage: forge [options] [variable=value] [command] ... \n";
            std::cout << "Options: \n";
            command_line_parser.print( stdout );
            assignments_and_commands.push_back( "help" );
        }

        if ( root_directory.empty() )
        {
            root_directory = search_up_for_root_directory( directory, build_script ).generic_string();
            error_policy.error( root_directory.empty(), "The file '%s' could not be found to identify the root directory", build_script.c_str() );
        }

        if ( !root_directory.empty() )
        {
            Forge forge( directory, error_policy, &event_sink );
            forge.set_stack_trace_enabled( stack_trace_enabled );
            forge.set_root_directory( root_directory );
            bool executed_command = false;
            vector<string> assignments;

            vector<string>::const_iterator i = assignments_and_commands.begin();
            while ( i != assignments_and_commands.end() && error_policy.errors() == 0 )
            {
                string::size_type position = i->find( "=" );
                if ( position == string::npos )
                {
                    const string& command = *i;
                    forge.command( assignments, build_script, command );
                    executed_command = true;
                }
                else
                {
                    const string& assignment = *i;
                    assignments.push_back( assignment );
                }
                ++i;
            }
            
            if ( !executed_command && !version )
            {
                const string DEFAULT_COMMAND = "default";
                forge.command( assignments, build_script, DEFAULT_COMMAND );
            }            
        }

        return error_policy.errors();
    }

    catch ( const std::exception& exception )
    {
        fprintf( stderr, "forge: %s.\n", exception.what() );
        return EXIT_FAILURE;
    }

    catch ( ... )
    {
        fprintf( stderr, "forge: An unexpected error occured.\n" );
        return EXIT_FAILURE;
    }

    return EXIT_FAILURE;
}
