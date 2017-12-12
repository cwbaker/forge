
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/BuildToolEventSink.hpp>
#include <sweet/build_tool/ScriptInterface.hpp>
#include <sweet/lua/Lua.hpp>
#include <unit/UnitTest.h>

using namespace sweet::build_tool;

SUITE( TestPreorder )
{
    TEST_FIXTURE( ErrorChecker, error_from_lua_in_preorder_visit_is_reported_and_handled )
    {
        const char* script = 
            "ErrorInPreorderVisit = TargetPrototype { 'ErrorInPreorderVisit', BIND_PHONY }; \n"
            "local error_in_preorder_visit = target( 'error_in_preorder_visit', ErrorInPreorderVisit ); \n"
            "preorder( function(target) error('Error in preorder visit') end, error_in_preorder_visit ); \n"
        ;        
        test( script );
        CHECK_EQUAL( "[string \"BuildTool\"]:3: Error in preorder visit", messages[0] );
        CHECK_EQUAL( "Preorder visit of 'error_in_preorder_visit' failed", messages[1] );
        CHECK( errors == 2 );
    }
    
    TEST_FIXTURE( ErrorChecker, unexpected_error_from_lua_in_preorder_visit_is_reported_and_handled )
    {
        const char* script = 
            "UnexpectedErrorInPreorderVisit = TargetPrototype { 'UnexpectedErrorInPreorderVisit', BIND_PHONY }; \n"
            "local unexpected_error_in_preorder_visit = target( 'unexpected_error_in_preorder_visit', UnexpectedErrorInPreorderVisit ); \n"
            "preorder( function(target) foo.bar = 2; end, unexpected_error_in_preorder_visit ); \n"
        ;        
        test( script );
        CHECK_EQUAL( "[string \"BuildTool\"]:3: attempt to index global 'foo' (a nil value)", messages[0] );
        CHECK_EQUAL( "Preorder visit of 'unexpected_error_in_preorder_visit' failed", messages[1] );
        CHECK( errors == 2 );
    }
    
    TEST_FIXTURE( ErrorChecker, recursive_preorder_is_reported_and_handled )
    {
        const char* script = 
            "RecursivePreorderError = TargetPrototype { 'RecursivePreorderError', BIND_PHONY }; \n"
            "local recursive_preorder_error = target( 'recursive_preorder_error', RecursivePreorderError ); \n"
            "preorder( function(target) preorder(function(target) end, recursive_preorder_error) end, recursive_preorder_error ); \n"
        ;
        test( script );
        CHECK_EQUAL( "Preorder called from within another preorder or postorder traversal", messages[0] );
        CHECK_EQUAL( "Preorder visit of 'recursive_preorder_error' failed", messages[1] );
        CHECK( errors == 2 );
    }

    TEST_FIXTURE( ErrorChecker, recursive_postorder_is_reported_and_handled )
    {
        const char* script = 
            "RecursivePostorderError = TargetPrototype { 'RecursivePostorderError', BIND_PHONY }; \n"
            "local recursive_postorder_error = target( 'recursive_postorder_error', RecursivePostorderError ); \n"
            "preorder( function(target) postorder(function(target) end, recursive_postorder_error) end, recursive_postorder_error ); \n"
        ;
        test( script );
        CHECK_EQUAL( "Postorder called from within another preorder or postorder traversal", messages[0] );
        CHECK_EQUAL( "Preorder visit of 'recursive_postorder_error' failed", messages[1] );
        CHECK( errors == 2 );
    }
}
