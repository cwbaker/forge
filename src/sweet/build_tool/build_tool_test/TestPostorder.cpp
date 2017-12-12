
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/BuildToolEventSink.hpp>
#include <sweet/build_tool/ScriptInterface.hpp>
#include <sweet/lua/Lua.hpp>
#include <unit/UnitTest.h>

using namespace sweet::build_tool;

SUITE( TestPostorder )
{
    TEST_FIXTURE( ErrorChecker, error_from_lua_in_postorder_visit_is_reported_and_handled )
    {
        const char* script = 
            "ErrorInPostorderVisit = TargetPrototype { 'ErrorInPostorderVisit', BIND_PHONY }; \n"
            "local error_in_postorder_visit = target( 'error_in_postorder_visit', ErrorInPostorderVisit ); \n"
            "postorder( function(target) error('Error in postorder visit') end, error_in_postorder_visit ); \n"
        ;        
        test( script );
        CHECK_EQUAL( "[string \"BuildTool\"]:3: Error in postorder visit", messages[0] );
        CHECK_EQUAL( "Postorder visit of 'error_in_postorder_visit' failed", messages[1] );
        CHECK( errors == 2 );
    }
    
    TEST_FIXTURE( ErrorChecker, unexpected_error_from_lua_in_postorder_visit_is_reported_and_handled )
    {
        const char* script = 
            "UnexpectedErrorInPostorderVisit = TargetPrototype { 'UnexpectedErrorInPostorderVisit', BIND_PHONY }; \n"
            "local unexpected_error_in_postorder_visit = target( 'unexpected_error_in_postorder_visit', UnexpectedErrorInPostorderVisit ); \n"
            "postorder( function(target) foo.bar = 2; end, unexpected_error_in_postorder_visit ); \n"
        ;        
        test( script );
        CHECK_EQUAL( "[string \"BuildTool\"]:3: attempt to index a nil value (global 'foo')", messages[0] );
        CHECK_EQUAL( "Postorder visit of 'unexpected_error_in_postorder_visit' failed", messages[1] );
        CHECK( errors == 2 );
    }
    
    TEST_FIXTURE( ErrorChecker, recursive_postorder_is_reported_and_handled )
    {
        const char* script = 
            "RecursivePostorderError = TargetPrototype { 'RecursivePostorderError', BIND_PHONY }; \n"
            "local recursive_postorder_error = target( 'recursive_postorder_error', RecursivePostorderError ); \n"
            "postorder( function(target) postorder(function(target) end, recursive_postorder_error) end, recursive_postorder_error ); \n"
        ;
        test( script );
        CHECK_EQUAL( "Postorder called from within another bind or postorder traversal", messages[0] );
        CHECK_EQUAL( "Postorder visit of 'recursive_postorder_error' failed", messages[1] );
        CHECK( errors == 2 );
    }

    TEST_FIXTURE( ErrorChecker, recursive_postorder_during_postorder_is_reported_and_handled )
    {
        const char* script = 
            "RecursivePostorderError = TargetPrototype { 'RecursivePostorderError', BIND_PHONY }; \n"
            "local recursive_postorder_error = target( 'recursive_postorder_error', RecursivePostorderError ); \n"
            "postorder( function(target) postorder(function(target) end, recursive_postorder_error) end, recursive_postorder_error ); \n"
        ;
        test( script );
        CHECK_EQUAL( "Postorder called from within another bind or postorder traversal", messages[0] );
        CHECK_EQUAL( "Postorder visit of 'recursive_postorder_error' failed", messages[1] );
        CHECK( errors == 2 );
    }
}
