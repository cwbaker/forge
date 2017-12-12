
package.path = root("build/lua/?.lua")..";"..root("build/lua/?/init.lua");
require "build";
require "build/clang";
require "build/mingw";
require "build/msvc";
require "build/boost";
require "build/Parser";
require "build/visual_studio";
require "build/xcode";

function initialize()
    platform = platform or build.switch { operating_system(); windows = "msvc"; macosx = "clang"; };
    variant = lower( variant or "debug" );
    version = version or os.date( "%Y.%m.%d %H:%M:%S "..platform.." "..variant );
    goal = goal or "";
    jobs = jobs or 4;

    local settings = build.initialize {
        bin = root( "../%s_%s/bin" % {platform, variant} );
        lib = root( "../%s_%s/lib" % {platform, variant} );
        obj = root( "../%s_%s/obj" % {platform, variant} );
        include_directories = {
            root(),
            root( "boost" )
        };
        library_directories = {
            root( "../%s_%s/lib" % {platform, variant} ),
        };
        sln = root( "../sweet_build_tool.sln" );
        xcodeproj = root( "../sweet_build_tool.xcodeproj" );
    };

    if operating_system() == "windows" then    
        mingw.initialize( settings );        
        msvc.initialize( settings );
        visual_studio.initialize( settings );
    elseif operating_system() == "macosx" then
        clang.initialize( settings );
        xcode.initialize( settings );
    end

    boost.initialize( settings );
    parser.initialize( settings );
end

function buildfiles()
    buildfile( "boost/boost.build" );
    buildfile( "lua/lua.build" );
    buildfile( "sweet/assert/assert.build" );
    buildfile( "sweet/atomic/atomic.build" );
    buildfile( "sweet/build_tool/build_tool.build" );
    buildfile( "sweet/cmdline/cmdline.build" );
    buildfile( "sweet/error/error.build" );
    buildfile( "sweet/lua/lua.build" );
    buildfile( "sweet/path/path.build" );
    buildfile( "sweet/persist/persist.build" );
    buildfile( "sweet/pointer/pointer.build" );
    buildfile( "sweet/process/process.build" );
    buildfile( "sweet/rtti/rtti.build" );
    buildfile( "sweet/thread/thread.build" );
    buildfile( "sweet/traits/traits.build" );
    buildfile( "unit/unit.build" );
end
