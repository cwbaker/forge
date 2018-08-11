//
// TestGraph.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include "FileChecker.hpp"
#include <UnitTest++/UnitTest++.h>

using namespace sweet::forge;

SUITE( TestGraph )
{
    TEST_FIXTURE( FileChecker, files_are_outdated_if_they_do_not_exist )
    {
        const char* script = 
            "local foo_cpp = forge:file( 'foo.cpp' ); \n"
            "local foo_obj = forge:file( 'foo.obj' ); \n"
            "foo_obj:add_dependency( foo_cpp ); \n"
            "forge:postorder( function() end, foo_obj ); \n"
            "assert( foo_obj:outdated() ); \n"
        ;
        create( "foo.cpp", "" );
        test( script );
        CHECK( errors == 0 );
    }
    
    TEST_FIXTURE( FileChecker, files_are_not_outdated_if_they_do_exist )
    {
        const char* script = 
            "local SourceFile = forge:target_prototype( 'SourceFile' ); \n"
            "local File = forge:target_prototype( 'File' ); \n"
            "local foo_cpp = forge:target( 'foo.cpp', SourceFile ); \n"
            "local foo_obj = forge:target( 'foo.obj', File ); \n"
            "foo_obj:add_dependency( foo_cpp ); \n"
            "forge:postorder( function() end, foo_obj ); \n"
            "assert( foo_obj:outdated() == false ); \n"
        ;
        create( "foo.cpp", "" );
        create( "foo.obj", "" );
        test( script );
        CHECK( errors == 0 );
    }
    
    TEST_FIXTURE( FileChecker, targets_are_outdated_if_their_dependencies_are_outdated )
    {
        const char* script = 
            "local foo_cpp = forge:file( 'foo.cpp' ); \n"
            "local foo_hpp = forge:file( 'foo.hpp' ); \n"
            "local foo_obj = forge:file( 'foo.obj' ); \n"
            "foo_cpp:add_dependency( foo_hpp ); \n"
            "foo_obj:add_dependency( foo_cpp ); \n"
            "forge:postorder( function() end, foo_obj ); \n"
            "assert( foo_obj:outdated() ); \n"
        ;
        create( "foo.cpp", "", 1 );
        create( "foo.hpp", "", 2 );
        create( "foo.obj", "", 1 );
        test( script );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( ErrorChecker, creating_the_same_target_with_different_prototypes_fails )
    {
        const char* expected_message = 
            "The target 'foo.cpp' has been created with prototypes 'SourceFile' and 'File'"
        ;
        const char* script =
            "local SourceFile = forge:target_prototype( 'SourceFile' ); \n"
            "local File = forge:target_prototype( 'File' ); \n"
            "forge:target( 'foo.cpp', SourceFile ); \n"
            "forge:target( 'foo.cpp', File ); \n"
        ;
        test( script );
        CHECK_EQUAL( expected_message, messages[0] );
        CHECK( errors == 1 );
    }

    TEST_FIXTURE( ErrorChecker, targets_are_children_of_the_working_directory_when_they_are_created )
    {
        const char* script =
            "local SourceFile = forge:target_prototype( 'File' ); \n"
            "local foo_cpp = forge:target( 'foo.cpp', SourceFile ); \n"
            "assert( foo_cpp:parent() == foo_cpp:working_directory() ); \n"
        ;
        test( script );
        CHECK( errors == 0 );
    }
}
