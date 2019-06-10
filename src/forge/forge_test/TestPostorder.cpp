
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <forge/Forge.hpp>
#include <forge/ForgeEventSink.hpp>
#include <UnitTest++/UnitTest++.h>

using namespace sweet::forge;

SUITE( TestPostorder )
{
    TEST_FIXTURE( ErrorChecker, error_from_lua_in_postorder_visit_is_reported_and_handled )
    {
        const char* script = 
            "local ErrorInPostorderVisit = add_target_prototype( 'ErrorInPostorderVisit' ); \n"
            "local error_in_postorder_visit = add_target( forge, 'error_in_postorder_visit', ErrorInPostorderVisit ); \n"
            "postorder( error_in_postorder_visit, function(target) error('Error in postorder visit') end ); \n"
        ;        
        test( script );
        CHECK_EQUAL( "[string \"local ErrorInPostorderVisit = add_target_prot...\"]:3: Error in postorder visit", messages[0] );
        CHECK_EQUAL( "Postorder visit of 'error_in_postorder_visit' failed", messages[1] );
        CHECK( errors == 2 );
    }
    
    TEST_FIXTURE( ErrorChecker, unexpected_error_from_lua_in_postorder_visit_is_reported_and_handled )
    {
        const char* script = 
            "local UnexpectedErrorInPostorderVisit = add_target_prototype( 'UnexpectedErrorInPostorderVisit' ); \n"
            "local unexpected_error_in_postorder_visit = add_target( forge, 'unexpected_error_in_postorder_visit', UnexpectedErrorInPostorderVisit ); \n"
            "postorder( unexpected_error_in_postorder_visit, function(target) foo.bar = 2; end ); \n"
        ;        
        test( script );
        if ( messages.size() == 2 )
        {
            CHECK_EQUAL( "[string \"local UnexpectedErrorInPostorderVisit = add_t...\"]:3: attempt to index a nil value (global 'foo')", messages[0] );
            CHECK_EQUAL( "Postorder visit of 'unexpected_error_in_postorder_visit' failed", messages[1] );
        }
        CHECK( errors == 2 );
    }
    
    TEST_FIXTURE( ErrorChecker, recursive_postorder_is_reported_and_handled )
    {
        const char* script = 
            "local RecursivePostorderError = add_target_prototype( 'RecursivePostorderError' ); \n"
            "local recursive_postorder_error = add_target( forge, 'recursive_postorder_error', RecursivePostorderError ); \n"
            "postorder( recursive_postorder_error, function(target) postorder(function(target) end, recursive_postorder_error) end ); \n"
        ;
        test( script );
        if ( messages.size() == 2 )
        {
            CHECK_EQUAL( "[string \"local RecursivePostorderError = add_target_pr...\"]:3: Postorder called from within another bind or postorder traversal", messages[0] );
            CHECK_EQUAL( "Postorder visit of 'recursive_postorder_error' failed", messages[1] );
        }
        CHECK( errors == 2 );
    }

    TEST_FIXTURE( ErrorChecker, recursive_postorder_during_postorder_is_reported_and_handled )
    {
        const char* script = 
            "local RecursivePostorderError = add_target_prototype( 'RecursivePostorderError' ); \n"
            "local recursive_postorder_error = add_target( forge, 'recursive_postorder_error', RecursivePostorderError ); \n"
            "postorder( recursive_postorder_error, function(target) postorder(function(target) end, recursive_postorder_error) end ); \n"
        ;
        test( script );
        if ( messages.size() == 2 )
        {
            CHECK_EQUAL( "[string \"local RecursivePostorderError = add_target_pr...\"]:3: Postorder called from within another bind or postorder traversal", messages[0] );
            CHECK_EQUAL( "Postorder visit of 'recursive_postorder_error' failed", messages[1] );
        }
        CHECK( errors == 2 );
    }
}
