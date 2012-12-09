
package.path = root("build/lua/?.lua")..";"..root("build/lua/?/init.lua");
require "build";
require "build/llvmgcc";
require "build/mingw";
require "build/msvc";
require "build/boost";
require "build/Parser";
require "build/visual_studio";
require "build/xcode";

function initialize()
    platform = platform or build.switch { operating_system(); windows = "msvc"; macosx = "llvmgcc"; };
    variant = lower( variant or "debug" );
    version = version or os.date( "%Y.%m.%d %H:%M:%S "..platform.." "..variant );
    goal = goal or "";
    jobs = jobs or 4;

    local boost_include_directory, boost_library_directory;
    if operating_system() == "windows" then
        boost_include_directory = "C:/boost/include/boost-1_43";
        boost_library_directory = "C:/boost/lib";
    elseif operating_system() == "macosx" then
        boost_include_directory = home( "boost/include/boost-1_43" );
        boost_library_directory = home( "boost/lib" );
    end

    local settings = build.initialize {
        bin = root( "../bin" );
        lib = root( "../lib" );
        obj = root( "../obj" );
        include_directories = {
            root( ".." ),
            boost_include_directory
        };
        library_directories = {
            boost_library_directory
        };
        sln = root( "../sweet_build_tool.sln" );
        xcodeproj = root( "../sweet_build_tool" );
    };

    if operating_system() == "windows" then    
        mingw.initialize( settings );        
        msvc.initialize( settings );
        visual_studio.initialize( settings );
    elseif operating_system() == "macosx" then
        llvmgcc.initialize( settings );
        xcode.initialize( settings );
    end

    boost.initialize( settings );
    parser.initialize( settings );
end

function buildfiles()
    buildfile( "assert/assert.build" );
    buildfile( "atomic/atomic.build" );
    buildfile( "build_tool/build_tool.build" );
    buildfile( "cmdline/cmdline.build" );
    buildfile( "error/error.build" );
    buildfile( "lua/lua.build" );
    buildfile( "path/path.build" );
    buildfile( "persist/persist.build" );
    buildfile( "pointer/pointer.build" );
    buildfile( "process/process.build" );
    buildfile( "rtti/rtti.build" );
    buildfile( "thread/thread.build" );
    buildfile( "traits/traits.build" );
    buildfile( "unit/unit.build" );
end
