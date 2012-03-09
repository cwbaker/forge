//
// Application.cpp
// Copyright (c) 2008 - 2012 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "Application.hpp"
#include <sweet/process/Process.hpp>
#include <sweet/cmdline/Parser.hpp>
#include <stdlib.h>
#include <stdio.h>

using namespace sweet;
using namespace sweet::process;

/**
// Constructor.
*/
Application::Application( int argc, char** argv )
: m_result( EXIT_FAILURE )
{
    bool help = false;
    bool version = false;
    bool print_to_stdout = false;
    bool print_to_stderr = false;

    cmdline::Parser command_line_parser;
    command_line_parser.add_options()
        ( "help",    "h", "Print this message and exit", &help            )
        ( "version", "v", "Print the version and exit",  &version         )
        ( "stdout",  "",  "Print output to stdout",      &print_to_stdout )
        ( "stderr",  "",  "Print output to stderr",      &print_to_stderr )
    ;
    command_line_parser.parse( argc, argv );

    if ( version )
    {
        fprintf( stdout, "Sweet Process Test " BUILD_VERSION " \n" );
        fprintf( stdout, "Copyright (c) 2007 - 2012 Charles Baker.  All rights reserved. \n" );
    }
    else if ( help )
    {
        fprintf( stdout, "Usage: test [options] \n" );
        fprintf( stdout, "Options: \n" );
        command_line_parser.print( stdout );
    }
    else if ( !print_to_stdout && !print_to_stderr )
    {
        test_main();
    }
    else
    {
        printf( "stdout=%s, stderr=%s\n", print_to_stdout ? "true" : "false", print_to_stderr ? "true" : "false" );

        if ( print_to_stdout )
        {
            test_print_to_stdout();
        }

        if ( print_to_stderr )
        {
            test_print_to_stderr();
        }
    }
}

/**
// Get the result to return to the operating system.
//
// @return
//  EXIT_SUCCESS if the tests ran successfully otherwise EXIT_FAILURE.
*/
int Application::result() const
{
    return m_result;
}

/**
// Test executing child processes and reading from stdout and stderr.
//
// @return
//  Nothing.
*/
void Application::test_main()
{    
//
// Test reading from the stdout of a child process.
//
    {
        process::Process process( "test.exe", "test --stdout", ".", PROCESS_FLAG_PROVIDE_STDOUT_AND_STDERR );

        char buffer [1024];        
        size_t read = process.read( buffer, sizeof(buffer) );
        while ( read > 0 )
        {
            fwrite( buffer, sizeof(char), read, stdout );
            read = process.read( buffer, sizeof(buffer) );
        }

        process.wait();
        if ( process.exit_code() == EXIT_SUCCESS )
        {
            m_result = EXIT_SUCCESS;
        }
    }    

//
// Test reading from the stderr of a child process.
//
    {
        process::Process process( "test.exe", "test --stderr", ".", PROCESS_FLAG_PROVIDE_STDOUT_AND_STDERR );

        char buffer [1024];        
        size_t read = process.read( buffer, sizeof(buffer) );
        while ( read > 0 )
        {
            fwrite( buffer, sizeof(char), read, stdout );
            read = process.read( buffer, sizeof(buffer) );
        }

        process.wait();
        if ( m_result == EXIT_SUCCESS && process.exit_code() == EXIT_SUCCESS )
        {
            m_result = EXIT_SUCCESS;
        }
    }    

//
// Test reading from both stdout and stderr of a child process.
//
    {
        process::Process process( "test.exe", "test --stdout --stderr", ".", PROCESS_FLAG_PROVIDE_STDOUT_AND_STDERR );

        char buffer [1024];        
        size_t read = process.read( buffer, sizeof(buffer) );
        while ( read > 0 )
        {
            fwrite( buffer, sizeof(char), read, stdout );
            read = process.read( buffer, sizeof(buffer) );
        }

        process.wait();
        if ( m_result == EXIT_SUCCESS && process.exit_code() == EXIT_SUCCESS )
        {
            m_result = EXIT_SUCCESS;
        }
    }    
}

/**
// Print output to stdout for the parent process to read.
//
// @return
//  Nothing.
*/
void Application::test_print_to_stdout()
{
    for ( int i = 0; i < 16; ++i )
    {
        fprintf( stdout, "This is line %d to stdout\n", i );
    }

    m_result = EXIT_SUCCESS;
}

/**
// Print output to stderr for the parent process to read.
//
// @return
//  Nothing.
*/
void Application::test_print_to_stderr()
{
    for ( int i = 0; i < 16; ++i )
    {
        fprintf( stderr, "This is line %d to stderr\n", i );
    }

    m_result = EXIT_SUCCESS;
}
