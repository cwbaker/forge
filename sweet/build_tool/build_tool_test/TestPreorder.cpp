
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/BuildToolEventSink.hpp>
#include <sweet/build_tool/ScriptInterface.hpp>
#include <sweet/lua/Lua.hpp>
#include <sweet/unit/UnitTest.h>

using namespace sweet::build_tool;

SUITE( TestPreorder )
{
    TEST_FIXTURE( ErrorChecker, error_from_lua_in_preorder_visit_is_reported_and_handled )
    {
        const char* expected_message =
            "[string \"BuildTool\"]:3: Error in preorder visit (in preorder for 'error_in_preorder_visit')"
        ;
        const char* script = 
            "ErrorInPreorderVisit = Rule( \"ErrorInPreorderVisit\", BIND_PHONY ); \n"
            "local error_in_preorder_visit = ErrorInPreorderVisit( 'error_in_preorder_visit' ); \n"
            "preorder( function(target) error('Error in preorder visit') end, error_in_preorder_visit ); \n"
        ;        
        test( script );
        CHECK_EQUAL( expected_message, message );
        CHECK( errors == 1 );
    }
    
    TEST_FIXTURE( ErrorChecker, unexpected_error_from_lua_in_preorder_visit_is_reported_and_handled )
    {
        const char* expected_message =
            "[string \"BuildTool\"]:3: attempt to index global 'foo' (a nil value) (in preorder for 'unexpected_error_in_preorder_visit')"
        ;
        const char* script = 
            "UnexpectedErrorInPreorderVisit = Rule( \"UnexpectedErrorInPreorderVisit\", BIND_PHONY ); \n"
            "local unexpected_error_in_preorder_visit = UnexpectedErrorInPreorderVisit( 'unexpected_error_in_preorder_visit' ); \n"
            "preorder( function(target) foo.bar = 2; end, unexpected_error_in_preorder_visit ); \n"
        ;        
        test( script );
        CHECK_EQUAL( expected_message, message );
        CHECK( errors == 1 );
    }
    
    TEST_FIXTURE( ErrorChecker, recursive_preorder_is_reported_and_handled )
    {
        const char* expected_message =
            "Preorder called from within another preorder or postorder traversal (in preorder for 'recursive_preorder_error')"
        ;
        const char* script = 
            "RecursivePreorderError = Rule( \"RecursivePreorderError\", BIND_PHONY ); \n"
            "local recursive_preorder_error = RecursivePreorderError( 'recursive_preorder_error' ); \n"
            "preorder( function(target) preorder(function(target) end, recursive_preorder_error) end, recursive_preorder_error ); \n"
        ;
        test( script );
        CHECK_EQUAL( expected_message, message );
        CHECK( errors == 1 );
    }

    TEST_FIXTURE( ErrorChecker, recursive_postorder_is_reported_and_handled )
    {
        const char* expected_message =
            "Postorder called from within another preorder or postorder traversal (in preorder for 'recursive_postorder_error')"
        ;
        const char* script = 
            "RecursivePostorderError = Rule( \"RecursivePostorderError\", BIND_PHONY ); \n"
            "local recursive_postorder_error = RecursivePostorderError( 'recursive_postorder_error' ); \n"
            "preorder( function(target) postorder(function(target) end, recursive_postorder_error) end, recursive_postorder_error ); \n"
        ;
        test( script );
        CHECK_EQUAL( expected_message, message );
        CHECK( errors == 1 );
    }
}
