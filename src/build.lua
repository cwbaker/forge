
package.path = build:root("build/lua/?.lua")..";"..build:root("build/lua/?/init.lua");
require "build";
require "build.cc";
require "build.parser";
require "build.visual_studio";
require "build.xcode";
require "build.macosx";
require "build.windows";

platform = platform or build:operating_system();
variant = build:lower( variant or "debug" );
version = version or ("%s %s %s"):format( os.date("%Y.%m.%d %H:%M:%S"), platform, variant );
goal = goal or "";
jobs = jobs or 4;

local settings = build:initialize {
    variants = { "debug", "release", "shipping" };
    bin = build:root( ("../%s/bin"):format(variant) );
    lib = build:root( ("../%s/lib"):format(variant) );
    obj = build:root( ("../%s/obj"):format(variant) );
    include_directories = {
        build:root(),
        build:root( "boost" )
    };
    library_directories = {
        build:root( ("../%s/lib"):format(variant) ),
    };
    visual_studio = {
        sln = build:root( "../sweet_build_tool.sln" );
    };
    xcode = {
        xcodeproj = build:root( "../sweet_build_tool.xcodeproj" );
    };
    zero_brane_studio = {
        mobdebug = build:switch { build:operating_system();
            macosx = "/Applications/ZeroBraneStudio.app/Contents/ZeroBraneStudio/lualibs/mobdebug/mobdebug.lua";
            windows = "C:\\Program Files (x86)\\ZeroBraneStudio\\lualibs\\mobdebug\\mobdebug.lua";
        };
    };
};

-- Targets built when building from the root directory and as targets when
-- generating XCode projects and Visual Studio solutions.
build:default_targets {
    "sweet/build_tool/build",
    "sweet/build_tool/build_hooks",
    "sweet/build_tool/build_tool_test"
};

buildfile "boost/boost.build";
buildfile "lua/lua.build";
buildfile "sweet/assert/assert.build";
buildfile "sweet/build_tool/build_tool.build";
buildfile "sweet/cmdline/cmdline.build";
buildfile "sweet/error/error.build";
buildfile "sweet/lua/lua.build";
buildfile "sweet/persist/persist.build";
buildfile "sweet/process/process.build";
buildfile "sweet/rtti/rtti.build";
buildfile "sweet/traits/traits.build";
buildfile "unit/unit.build";
