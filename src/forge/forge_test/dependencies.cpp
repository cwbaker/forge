//
// TestTransitiveDependencies.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "stdafx.hpp"
#include <forge/Forge.hpp>
#include <forge/ForgeEventSink.hpp>
#include <error/ErrorPolicy.hpp>
#include <luaxx/luaxx_unit/LuaUnitTest.hpp>
#include <assert/assert.hpp>
#include <UnitTest++/UnitTest++.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <string>

using std::string;
using namespace sweet;
using namespace sweet::forge;
using namespace luaxx;
using namespace boost::filesystem;

SUITE( dependencies )
{
    struct LuaTest : public forge::ForgeEventSink
    {
        error::ErrorPolicy* error_policy_;
        forge::Forge* forge_;
        luaxx::LuaUnitTest* lua_unit_test_;
    
        LuaTest()
        {
            path path = boost::filesystem::path( string(TEST_DIRECTORY) );
            error_policy_ = new error::ErrorPolicy;
            forge_ = new forge::Forge( path.string(), *error_policy_, this );
            lua_unit_test_ = new LuaUnitTest( forge_->lua_state(), error_policy_ );
            forge_->set_root_directory( path.generic_string() );
            forge_->set_stack_trace_enabled( true );
            forge_->set_package_path(
                forge_->root("../lua/?.lua").generic_string() + ";" +
                forge_->root("../lua/?/init.lua").generic_string()
            );
        }

        ~LuaTest()
        {
            delete lua_unit_test_;
            delete forge_;
            delete error_policy_;
        }

        void forge_output( Forge* /*forge*/, const char* message )
        {
            SWEET_ASSERT( message );
            fputs( message, stdout );
            fputs( "\n", stdout );
            fflush( stdout );
        }

        void forge_warning( Forge* /*forge*/, const char* message )
        {
            SWEET_ASSERT( message );            
            fputs( "forge_test: ", stderr );
            fputs( message, stderr );
            fputs( ".\n", stderr );
            fflush( stderr );
        }

        void forge_error( Forge* /*forge*/, const char* message )
        {
            SWEET_ASSERT( message );            
            fputs( "forge_test: ", stderr );
            fputs( message, stderr );
            fputs( ".\n", stderr );
            fflush( stderr );
        }
    };

    TEST_FIXTURE( LuaTest, transitive_dependencies )
    {
        forge_->file( "transitive_dependencies.lua" );
    }
}
