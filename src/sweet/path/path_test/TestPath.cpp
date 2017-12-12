//
// TestPath.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/path/path.hpp>

using namespace sweet::path;

SUITE( TestPath )
{
    TEST( TestDrive )
    {
        Path drive_test( "D:/sweet/persist-0.0.1/path/src/test" );
        CHECK( drive_test.string() == "D:/sweet/persist-0.0.1/path/src/test" );
        CHECK( drive_test.branch().string() == "D:/sweet/persist-0.0.1/path/src" );
        CHECK( drive_test.drive() == "D:" );
        CHECK( drive_test.leaf() == "test" );
        CHECK( drive_test.basename() == "test" );
        CHECK( drive_test.extension() == "" );
        CHECK( drive_test.is_absolute() );
        CHECK( !drive_test.is_relative() );
    }

    TEST( TestRoot )
    {
        Path root_test( "/sweet/sweet-persist-0.0.1/path/src/test/zzcb.xml" );
        CHECK( root_test.string() == "/sweet/sweet-persist-0.0.1/path/src/test/zzcb.xml" );
        CHECK( root_test.branch().string() == "/sweet/sweet-persist-0.0.1/path/src/test" );
        CHECK( root_test.leaf() == "zzcb.xml" );
        CHECK( root_test.basename() == "zzcb" );
        CHECK( root_test.extension() == ".xml" );
        CHECK( root_test.is_absolute() );
        CHECK( !root_test.is_relative() );
    }

    TEST( TestRelative )
    {
        Path relative_test( "path/src/test" );
        CHECK( relative_test.string() == "path/src/test" );
        CHECK( relative_test.branch().string() == "path/src" );
        CHECK( relative_test.leaf() == "test" );
        CHECK( relative_test.basename() == "test" );
        CHECK( relative_test.extension() == "" );
        CHECK( !relative_test.is_absolute() );
        CHECK( relative_test.is_relative() );
    }

    TEST( TestBaseAndRelated )
    {
        Path base_path( "/sweet/rw/data" );
        Path related_path( "/sweet/rw/cfg/rw.cfg" );
        Path relative_path = base_path.relative( related_path );
        CHECK( relative_path.string() == "../cfg/rw.cfg" );

        related_path = Path( "/sweet/rw/data/items/short-sword.xml" );
        relative_path = base_path.relative( related_path );
        CHECK( relative_path.string() == "items/short-sword.xml" );

        related_path = Path( "/sweet/rw.xml" );
        relative_path = base_path.relative( related_path );
        CHECK( relative_path.string() == "../../rw.xml" );

        base_path = Path( "D:/some/other/path" );
        related_path = Path( "C:/foo/blah.txt" );
        relative_path = base_path.relative( related_path );
        CHECK( relative_path.string() == "C:/foo/blah.txt" );
    }

    TEST( TestEmpty )
    {
        Path empty_test;
        CHECK( empty_test.string() == "" );
        CHECK( empty_test.native_string() == "" );
        CHECK( empty_test.branch().string() == "" );
        CHECK( empty_test.leaf() == "" );
        CHECK( empty_test.basename() == "" );
        CHECK( empty_test.extension() == "" );
    }

    TEST( TestNormalize )
    {
        Path normalize_test = Path( "/this/../is/../a/../../non/normalized/path/../../with/../some/stuff/that/will/be/removed" );
        normalize_test.normalize();
        CHECK( normalize_test.string() == "/../non/some/stuff/that/will/be/removed" );

        normalize_test = Path( "/this/has/some/../../../stuff/only/in/the/middle" );
        normalize_test.normalize();
        CHECK( normalize_test.string() == "/stuff/only/in/the/middle" );
    }

    TEST( TestNative )
    {
        Path drive_test( "D:/sweet/persist-0.0.1/path/src/test" );
        Path root_test( "/sweet/sweet-persist-0.0.1/path/src/test/zzcb.xml" );
        Path relative_test( "path/src/test" );

#if defined(BUILD_OS_WINDOWS)
        CHECK( drive_test.native_string() == "D:\\sweet\\persist-0.0.1\\path\\src\\test" );
        CHECK( root_test.native_string() == "\\sweet\\sweet-persist-0.0.1\\path\\src\\test\\zzcb.xml" );
        CHECK( relative_test.native_string() == "path\\src\\test" );        
#else
        printf( "%s\n", drive_test.native_string().c_str() );
        CHECK( drive_test.native_string() == "D:/sweet/persist-0.0.1/path/src/test" );
        CHECK( root_test.native_string() == "/sweet/sweet-persist-0.0.1/path/src/test/zzcb.xml" );
        CHECK( relative_test.native_string() == "path/src/test" );        
#endif
    }
}
