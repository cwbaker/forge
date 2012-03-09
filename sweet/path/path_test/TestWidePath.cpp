//
// TestWidePath.cpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#include "stdafx.hpp"
#include <sweet/unit/UnitTest.h>
#include <sweet/path/path.hpp>

using namespace sweet::path;

SUITE( TestWidePath )
{
    TEST( TestDrive )
    {
        WidePath drive_test( L"D:/sweet/persist-0.0.1/path/src/test" );
        CHECK( drive_test.string() == L"D:/sweet/persist-0.0.1/path/src/test" );
        CHECK( drive_test.native_string() == L"D:\\sweet\\persist-0.0.1\\path\\src\\test" );
        CHECK( drive_test.branch().string() == L"D:/sweet/persist-0.0.1/path/src" );
        CHECK( drive_test.drive() == L"D:" );
        CHECK( drive_test.leaf() == L"test" );
        CHECK( drive_test.basename() == L"test" );
        CHECK( drive_test.extension() == L"" );
        CHECK( drive_test.is_absolute() );
        CHECK( !drive_test.is_relative() );
    }

    TEST( TestRoot )
    {
        WidePath root_test( L"/sweet/sweet-persist-0.0.1/path/src/test/zzcb.xml" );
        CHECK( root_test.string() == L"/sweet/sweet-persist-0.0.1/path/src/test/zzcb.xml" );
        CHECK( root_test.native_string() == L"\\sweet\\sweet-persist-0.0.1\\path\\src\\test\\zzcb.xml" );
        CHECK( root_test.branch().string() == L"/sweet/sweet-persist-0.0.1/path/src/test" );
        CHECK( root_test.leaf() == L"zzcb.xml" );
        CHECK( root_test.basename() == L"zzcb" );
        CHECK( root_test.extension() == L".xml" );
        CHECK( root_test.is_absolute() );
        CHECK( !root_test.is_relative() );
    }

    TEST( TestRelative )
    {
        WidePath relative_test( L"path/src/test" );
        CHECK( relative_test.string() == L"path/src/test" );
        CHECK( relative_test.native_string() == L"path\\src\\test" );
        CHECK( relative_test.branch().string() == L"path/src" );
        CHECK( relative_test.leaf() == L"test" );
        CHECK( relative_test.basename() == L"test" );
        CHECK( relative_test.extension() == L"" );
        CHECK( !relative_test.is_absolute() );
        CHECK( relative_test.is_relative() );
    }

    TEST( TestBaseAndRelated )
    {
        WidePath base_path( L"/sweet/rw/data" );
        WidePath related_path( L"/sweet/rw/cfg/rw.cfg" );
        WidePath relative_path = base_path.relative( related_path );
        CHECK( relative_path.string() == L"../cfg/rw.cfg" );

        related_path = WidePath( L"/sweet/rw/data/items/short-sword.xml" );
        relative_path = base_path.relative( related_path );
        CHECK( relative_path.string() == L"items/short-sword.xml" );

        related_path = WidePath( L"/sweet/rw.xml" );
        relative_path = base_path.relative( related_path );
        CHECK( relative_path.string() == L"../../rw.xml" );

        base_path    = WidePath( L"D:/some/other/path" );
        related_path = WidePath( L"C:/foo/blah.txt" );
        relative_path = base_path.relative( related_path );
        CHECK( relative_path.string() == L"C:/foo/blah.txt" );
    }

    TEST( TestEmpty )
    {
        WidePath empty_test;
        CHECK( empty_test.string() == L"" );
        CHECK( empty_test.native_string() == L"" );
        CHECK( empty_test.branch().string() == L"" );
        CHECK( empty_test.leaf() == L"" );
        CHECK( empty_test.basename() == L"" );
        CHECK( empty_test.extension() == L"" );
    }

    TEST( TestNormalize )
    {
        WidePath normalize_test = WidePath( L"/this/../is/../a/../../non/normalized/path/../../with/../some/stuff/that/will/be/removed" );
        normalize_test.normalize();
        CHECK( normalize_test.string() == L"/../non/some/stuff/that/will/be/removed" );

        normalize_test = WidePath( L"/this/has/some/../../../stuff/only/in/the/middle" );
        normalize_test.normalize();
        CHECK( normalize_test.string() == L"/stuff/only/in/the/middle" );

        WidePath current_working_directory_test = current_working_directory();
    }
}
