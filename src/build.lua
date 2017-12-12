
package.path = root("build/lua/?.lua")..";"..root("build/lua/?/init.lua");
require "build";
require "build.cc";
require "build.parser";
require "build.visual_studio";
require "build.xcode";
require "build.macosx";
require "build.windows";

function initialize()
    platform = platform or operating_system();
    variant = lower( variant or "debug" );
    version = version or ("%s %s %s"):format( os.date("%Y.%m.%d %H:%M:%S"), platform, variant );
    goal = goal or "";
    jobs = jobs or 4;

    local settings = build.initialize {
        bin = root( ("../%s_%s/bin"):format(platform, variant) );
        lib = root( ("../%s_%s/lib"):format(platform, variant) );
        obj = root( ("../%s_%s/obj"):format(platform, variant) );
        include_directories = {
            root(),
            root( "boost" )
        };
        library_directories = {
            root( ("../%s_%s/lib"):format(platform, variant) ),
        };
        sln = root( "../sweet_build_tool.sln" );
        xcodeproj = {
            filename = root( "../sweet_build_tool.xcodeproj" );
            targets = {
                "sweet/build_tool/build",
                "sweet/build_tool/build_tool_test"
            };
        };
        zero_brane_studio = {
            mobdebug = build.switch { operating_system();
                macosx = "/Applications/ZeroBraneStudio.app/Contents/ZeroBraneStudio/lualibs/mobdebug/mobdebug.lua";
                windows = "C:\\Program Files (x86)\\ZeroBraneStudio\\lualibs\\mobdebug\\mobdebug.lua";
            };
        };
    };

    if operating_system() == "windows" then    
        windows.initialize( settings );  
        visual_studio.initialize( settings );
    elseif operating_system() == "macosx" then
        macosx.initialize( settings );
        xcode.initialize( settings );
    end

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
