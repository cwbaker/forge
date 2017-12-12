
package.path = root("build/lua/?.lua")..";"..root("build/lua/?/init.lua");
require "build";
require "build.cc";
require "build.parser";
require "build.visual_studio";
require "build.xcode";
require "build.macosx";
require "build.windows";

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
    buildfile = root( "build.build" );
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

build.default_buildfiles {
    "boost/boost.build",
    "lua/lua.build",
    "sweet/assert/assert.build",
    "sweet/atomic/atomic.build",
    "sweet/build_tool/build_tool.build",
    "sweet/cmdline/cmdline.build",
    "sweet/error/error.build",
    "sweet/lua/lua.build",
    "sweet/path/path.build",
    "sweet/persist/persist.build",
    "sweet/pointer/pointer.build",
    "sweet/process/process.build",
    "sweet/rtti/rtti.build",
    "sweet/thread/thread.build",
    "sweet/traits/traits.build",
    "unit/unit.build"
};

-- Targets built when building from the root directory and as targets when
-- generating XCode projects and Visual Studio solutions.
build.default_targets {
    "sweet/build_tool/build",
    "sweet/build_tool/build/build_hooks"
};
