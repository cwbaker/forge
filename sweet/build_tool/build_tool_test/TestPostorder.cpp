
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/BuildToolEventSink.hpp>
#include <sweet/build_tool/ScriptInterface.hpp>
#include <sweet/lua/Lua.hpp>
#include <sweet/unit/UnitTest.h>

using namespace sweet::build_tool;

SUITE( TestPostorder )
{
    TEST_FIXTURE( ErrorChecker, error_from_lua_in_postorder_visit_is_reported_and_handled )
    {
        const char* expected_message =
            "[string \"BuildTool\"]:3: Error in postorder visit (in postorder for 'error_in_postorder_visit')"
        ;
        const char* script = 
            "ErrorInPostorderVisit = Rule( \"ErrorInPostorderVisit\", BIND_PHONY ); \n"
            "local error_in_postorder_visit = ErrorInPostorderVisit( 'error_in_postorder_visit' ); \n"
            "postorder( function(target) error('Error in postorder visit') end, error_in_postorder_visit ); \n"
        ;        
        test( script );
        CHECK_EQUAL( expected_message, message );
        CHECK( errors == 1 );
    }
    
    TEST_FIXTURE( ErrorChecker, unexpected_error_from_lua_in_postorder_visit_is_reported_and_handled )
    {
        const char* expected_message =
            "[string \"BuildTool\"]:3: attempt to index global 'foo' (a nil value) (in postorder for 'unexpected_error_in_postorder_visit')"
        ;
        const char* script = 
            "UnexpectedErrorInPostorderVisit = Rule( \"UnexpectedErrorInPostorderVisit\", BIND_PHONY ); \n"
            "local unexpected_error_in_postorder_visit = UnexpectedErrorInPostorderVisit( 'unexpected_error_in_postorder_visit' ); \n"
            "postorder( function(target) foo.bar = 2; end, unexpected_error_in_postorder_visit ); \n"
        ;        
        test( script );
        CHECK_EQUAL( expected_message, message );
        CHECK( errors == 1 );
    }
    
    TEST_FIXTURE( ErrorChecker, recursive_postorder_is_reported_and_handled )
    {
        const char* expected_message =
            "Postorder called from within another preorder or postorder traversal (in postorder for 'recursive_postorder_error')"
        ;
        const char* script = 
            "RecursivePostorderError = Rule( \"RecursivePostorderError\", BIND_PHONY ); \n"
            "local recursive_postorder_error = RecursivePostorderError( 'recursive_postorder_error' ); \n"
            "postorder( function(target) postorder(function(target) end, recursive_postorder_error) end, recursive_postorder_error ); \n"
        ;
        test( script );
        CHECK_EQUAL( expected_message, message );
        CHECK( errors == 1 );
    }

    TEST_FIXTURE( ErrorChecker, recursive_postorder_during_postorder_is_reported_and_handled )
    {
        const char* expected_message =
            "Postorder called from within another preorder or postorder traversal (in postorder for 'recursive_postorder_error')"
        ;
        const char* script = 
            "RecursivePostorderError = Rule( \"RecursivePostorderError\", BIND_PHONY ); \n"
            "local recursive_postorder_error = RecursivePostorderError( 'recursive_postorder_error' ); \n"
            "postorder( function(target) postorder(function(target) end, recursive_postorder_error) end, recursive_postorder_error ); \n"
        ;
        test( script );
        CHECK_EQUAL( expected_message, message );
        CHECK( errors == 1 );
    }
}
