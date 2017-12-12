//
// TestGraph.cpp
// Copyright (c) 2010 - 2013 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include "FileChecker.hpp"
#include <sweet/unit/UnitTest.h>

using namespace sweet::build_tool;

SUITE( TestGraph )
{
    TEST_FIXTURE( FileChecker, files_are_outdated_if_they_do_not_exist )
    {
        const char* script = 
            "local foo_cpp = file( 'foo.cpp' ); \n"
            "local foo_obj = file( 'foo.obj' ); \n"
            "foo_obj:add_dependency( foo_cpp ); \n"
            "bind( foo_obj ); \n"
            "assert( foo_obj:is_outdated() ); \n"
        ;
        create( "foo.cpp", "" );
        test( script );
        CHECK( errors == 0 );
    }
    
    TEST_FIXTURE( FileChecker, files_are_not_outdated_if_they_do_exist )
    {
        const char* script = 
            "local SourceFilePrototype = TargetPrototype{ 'SourceFile', BIND_SOURCE_FILE }; \n"
            "local FilePrototype = TargetPrototype{ 'File', BIND_GENERATED_FILE }; \n"
            "local foo_cpp = target( 'foo.cpp', SourceFilePrototype ); \n"
            "local foo_obj = target( 'foo.obj', FilePrototype ); \n"
            "foo_obj:add_dependency( foo_cpp ); \n"
            "bind( foo_obj ); \n"
            "assert( foo_obj:is_outdated() == false ); \n"
        ;
        create( "foo.cpp", "" );
        create( "foo.obj", "" );
        test( script );
        CHECK( errors == 0 );
    }
    
    TEST_FIXTURE( FileChecker, targets_are_outdated_if_their_dependencies_are_outdated )
    {
        const char* script = 
            "local foo_cpp = file( 'foo.cpp' ); \n"
            "local foo_hpp = file( 'foo.hpp' ); \n"
            "local foo_obj = file( 'foo.obj' ); \n"
            "foo_cpp:add_dependency( foo_hpp ); \n"
            "foo_obj:add_dependency( foo_cpp ); \n"
            "bind( foo_obj ); \n"
            "assert( foo_obj:is_outdated() ); \n"
        ;
        create( "foo.cpp", "", 1 );
        create( "foo.hpp", "", 2 );
        create( "foo.obj", "", 1 );
        test( script );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( FileChecker, source_files_that_do_not_exist_generate_errors )
    {
        const char* script = 
            "local SourceFilePrototype = TargetPrototype { 'SourceFile', BIND_SOURCE_FILE }; \n"
            "local FilePrototype = TargetPrototype { 'File', BIND_GENERATED_FILE }; \n"
            "local foo_cpp = target( 'foo.cpp', SourceFilePrototype ); \n"
            "foo_cpp:set_required_to_exist( true ); \n"
            "bind( foo_cpp ); \n"
        ;
        test( script );
        CHECK( errors == 1 );
    }

    TEST_FIXTURE( FileChecker, source_files_that_exist_do_not_generate_errors )
    {
        const char* expected_message =
        {
            "The source file 'D:/sweet/sweet_build_tool/sweet/build_tool/build_tool_test/foo.cpp' does not exist",
        };
        const char* script = 
            "local foo_cpp = file( 'foo.cpp' ); \n"
            "foo_cpp:set_required_to_exist( true ); \n"
            "bind( foo_cpp ); \n"
        ;
        create( "foo.cpp", "" );
        test( script );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( FileChecker, header_files_that_are_removed_behave_correctly )
    {
        const char* script = 
            "local filename = 'header_files_that_are_removed_behave_correctly.cache'; \n"
            "local success = load_binary( filename ); \n"
            "assert( not success, 'Graph already exists' ); \n"
            "local foo_cpp = file( 'foo.cpp' ); \n"
            "foo_cpp:set_required_to_exist( true ); \n"
            "local foo_hpp = file( 'foo.hpp' ); \n"
            "foo_cpp:add_dependency( foo_hpp ); \n"
            "bind( foo_cpp ); \n"
            "save_binary( filename ); \n"
        ;
        remove( "header_files_that_are_removed_behave_correctly.cache" );
        create( "foo.cpp", "" );
        create( "foo.hpp", "" );
        test( script );
        CHECK( errors == 0 );

        const char* second_script = 
            "local filename = 'header_files_that_are_removed_behave_correctly.cache'; \n"
            "local success = load_binary( filename ); \n"
            "assert( success, 'Loading graph failed' ); \n"
            "local foo_cpp = file( 'foo.cpp' ); \n"
            "bind( foo_cpp ); \n"
        ;
        remove( "foo.hpp" );
        test( second_script );
        remove( "header_files_that_are_removed_behave_correctly.cache" );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( FileChecker, explicit_dependencies_are_not_removed_by_scan )
    {
        const char* script = 
            "local foo_hpp = file( 'foo.hpp' ); \n"
            "local foo_g = file( 'foo.g' ); \n"
            "foo_hpp:add_dependency( foo_g ); \n"
            "mark_implicit_dependencies(); \n"
            "local bar_hpp = file( 'bar.hpp' ); \n"
            "foo_hpp:add_dependency( bar_hpp ); \n"
            "local baz_hpp = file( 'baz.hpp' ); \n"
            "foo_hpp:add_dependency( baz_hpp ); \n"
            " \n"
            "local dependencies = {}; \n"
            "for dependency in foo_hpp:get_dependencies() do \n"
            "    table.insert( dependencies, dependency ); \n"
            "end \n"
            "assert( #dependencies == 3, '#dependencies == 3' ); \n"
            "assert( dependencies[1] == foo_g, 'foo.g' ); \n"
            "assert( dependencies[2] == bar_hpp, 'bar.hpp' ); \n"
            "assert( dependencies[3] == baz_hpp, 'baz.hpp' ); \n"
            " \n"
        ;                
        create( "foo.hpp", "" );
        test( script );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( ErrorChecker, creating_the_same_target_with_different_prototypes_fails )
    {
        const char* expected_message = 
            "The target 'foo.cpp' has been created with prototypes 'SourceFile' and 'File'"
        ;
        const char* script =
            "local SourceFilePrototype = TargetPrototype { 'SourceFile', BIND_SOURCE_FILE }; \n"
            "local FilePrototype = TargetPrototype { 'File', BIND_GENERATED_FILE }; \n"
            "target( 'foo.cpp', SourceFilePrototype ); \n"
            "target( 'foo.cpp', FilePrototype ); \n"
        ;
        test( script );
        CHECK_EQUAL( expected_message, messages[0] );
        CHECK( errors == 1 );
    }

    TEST_FIXTURE( ErrorChecker, targets_are_children_of_the_working_directory_when_they_are_created )
    {
        const char* script =
            "local SourceFilePrototype = TargetPrototype { 'File', BIND_SOURCE_FILE }; \n"
            "local foo_cpp = target( 'foo.cpp', SourceFilePrototype ); \n"
            "assert( foo_cpp:parent() == foo_cpp:get_working_directory() ); \n"
        ;
        test( script );
        CHECK( errors == 0 );
    }
}
