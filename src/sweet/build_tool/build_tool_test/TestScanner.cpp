
#include "stdafx.hpp"
#include "ErrorChecker.hpp"
#include "FileChecker.hpp"
#include <sweet/build_tool/BuildTool.hpp>
#include <sweet/build_tool/BuildToolEventSink.hpp>
#include <sweet/build_tool/ScriptInterface.hpp>
#include <sweet/lua/Lua.hpp>
#include <sweet/unit/UnitTest.h>
#include <vector>

using std::vector;
using namespace sweet::build_tool;

SUITE( TestScanner )
{
    TEST_FIXTURE( ErrorChecker, syntax_errors_in_regular_expression_fail_gracefully )
    {
        const char* expected_message =
            "Unmatched marking parenthesis ( or \\(.  The error occured while parsing the regular expression: 'abc(>>>HERE>>>'."
        ;
        const char* script = 
            "SyntaxErrorsInRegularExpression = Scanner { \n"
            "    [ [[abc(]] ] = function( target, match ) \n"
            "    end; \n"
            "} \n"
        ;        
        test( script );
        CHECK_EQUAL( expected_message, message );
        CHECK( errors == 1 );  
    }

    TEST_FIXTURE( FileChecker, local_includes_are_matched )
    {
        const char* CONTENTS =
            "#include \"local_include.hpp\" \n"
            " \n"
        ;
        
        const char* script = 
            "local matched = false; \n"
            " \n"
            "local_include_scanner = Scanner { \n"
            "   [ [[^#include \"([^\"\n\r]*)\"]] ] = function( target, match ) \n"
            "       matched = true; \n"
            "       assert( match == 'local_include.hpp' ); \n"
            "   end; \n"
            "}; \n"
            " \n"
            "local local_includes_are_matched = file( 'local_includes_are_matched' ); \n"
            "scan( local_includes_are_matched, local_include_scanner ); \n"
            "wait(); \n"
            "assert( matched, 'Local include failed to match' ); \n"
        ;
        
        create( "local_includes_are_matched", CONTENTS );
        test( script );
        CHECK( errors == 0 );
    }
    
    TEST_FIXTURE( FileChecker, global_includes_are_matched )
    {
        const char* CONTENTS =
            "#include <global_include.hpp> \n"
            " \n"
        ;
        
        const char* script = 
            "local matched = false; \n"
            " \n"
            "global_include_scanner = Scanner { \n"
            "   [ [[^#include <([^>\n\r]*)>]] ] = function( target, match ) \n"
            "       matched = true; \n"
            "       assert( match == 'global_include.hpp' ); \n"
            "   end; \n"
            "}; \n"
            " \n"
            "local global_includes_are_matched = file( 'global_includes_are_matched' ); \n"
            "scan( global_includes_are_matched, global_include_scanner ); \n"
            "wait(); \n"
            "assert( matched, 'Global include failed to match' ); \n"
        ;
        
        create( "global_includes_are_matched", CONTENTS );
        test( script );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( FileChecker, unchanged_files_are_not_scanned )
    {
        const char* CONTENTS =
            "Hello World! \n"
            " \n"
        ;
        
        const char* script = 
            "local matches = 0; \n"
            " \n"
            "hello_world_scanner = Scanner { \n"
            "   [ [[^Hello World!]] ] = function( target, match ) \n"
            "       matches = matches + 1; \n"
            "   end; \n"
            "}; \n"
            " \n"
            "local unchanged_files_are_not_scanned = file( 'unchanged_files_are_not_scanned' ); \n"
            "scan( unchanged_files_are_not_scanned, hello_world_scanner ); \n"
            "scan( unchanged_files_are_not_scanned, hello_world_scanner ); \n"
            "wait(); \n"
            "assert( matches == 1, 'Unchanged file scanned twice' ); \n"
        ;
        
        create( "unchanged_files_are_not_scanned", CONTENTS );
        test( script );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( FileChecker, initial_lines_setting_is_honored )
    {
        const char* CONTENTS =
            " \n"
            " \n"
            " \n"
            " \n"
            "Hello World! \n"
        ;
        
        const char* script = 
            "local matches = 0; \n"
            " \n"
            "local hello_world_scanner = Scanner { \n"
            "   [ [[^Hello World!]] ] = function( target, match ) \n"
            "       matches = matches + 1; \n"
            "   end; \n"
            "}; \n"
            " \n"
            "SourceFilePrototype = TargetPrototype { 'SourceFilePrototype', BIND_SOURCE_FILE }; \n"
            "local initial_lines_setting_is_honored = target( 'initial_lines_setting_is_honored', SourceFilePrototype ); \n"
            "hello_world_scanner:set_initial_lines( 2 ); \n"
            "scan( initial_lines_setting_is_honored, hello_world_scanner ); \n"
            "wait(); \n"
            "assert( matches == 0, 'Initial lines setting is not honored' ); \n"
        ;
        
        create( "initial_lines_setting_is_honored", CONTENTS );
        test( script );
        CHECK( errors == 0 );
    }

    TEST_FIXTURE( FileChecker, later_lines_setting_is_honored )
    {
        const char* CONTENTS =
            " \n"
            " \n"
            " \n"
            " \n"
            "Hello World! \n"
            " \n"
            " \n"
            " \n"
            " \n"
            "Hello World! \n"
        ;
        
        const char* script = 
            "local matches = 0; \n"
            " \n"
            "hello_world_scanner = Scanner { \n"
            "   [ [[^Hello World!]] ] = function( target, match ) \n"
            "       matches = matches + 1; \n"
            "   end; \n"
            "}; \n"
            " \n"
            "local later_lines_setting_is_honored = file( 'later_lines_setting_is_honored' ); \n"
            "hello_world_scanner:set_initial_lines( 5 ); \n"
            "hello_world_scanner:set_later_lines( 2 ); \n"
            "scan( later_lines_setting_is_honored, hello_world_scanner ); \n"
            "wait(); \n"
            "assert( matches == 1, 'Later lines setting is not honored' ); \n"
        ;
        
        create( "later_lines_setting_is_honored", CONTENTS );
        test( script );
        CHECK( errors == 0 );
    }
    
    TEST_FIXTURE( ErrorChecker, scanning_a_file_that_does_not_exist_fails_gracefully )
    {
        const char* script = 
            "local matches = 0; \n"
            " \n"
            "hello_world_scanner = Scanner { \n"
            "   [ [[^Hello World!]] ] = function( target, match ) \n"
            "       matches = matches + 1; \n"
            "   end; \n"
            "}; \n"
            " \n"
            "local scanning_a_file_that_does_not_exist_fails_gracefully = file( 'scanning_a_file_that_does_not_exist_fails_gracefully' ); \n"
            "scanning_a_file_that_does_not_exist_fails_gracefully:set_required_to_exist( true ); \n"
            "scan( scanning_a_file_that_does_not_exist_fails_gracefully, hello_world_scanner ); \n"
            "wait(); \n"
        ;        
        test( script );
        CHECK( errors == 1 );  
    }
}
