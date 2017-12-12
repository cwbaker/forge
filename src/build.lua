
package.path = build.root("build/lua/?.lua")..";"..build.root("build/lua/?/init.lua");
require "build";
require "build.cc";
require "build.parser";
require "build.visual_studio";
require "build.xcode";
require "build.macosx";
require "build.windows";

platform = platform or build.operating_system();
variant = build.lower( variant or "debug" );
version = version or ("%s %s %s"):format( os.date("%Y.%m.%d %H:%M:%S"), platform, variant );
goal = goal or "";
jobs = jobs or 4;

local settings = build.initialize {
    variants = { "debug", "release", "shipping" };
    bin = build.root( ("../%s_%s/bin"):format(platform, variant) );
    lib = build.root( ("../%s_%s/lib"):format(platform, variant) );
    obj = build.root( ("../%s_%s/obj"):format(platform, variant) );
    include_directories = {
        build.root(),
        build.root( "boost" )
    };
    library_directories = {
        build.root( ("../%s_%s/lib"):format(platform, variant) ),
    };
    visual_studio = {
        sln = build.root( "../sweet_build_tool.sln" );
    };
    xcode = {
        xcodeproj = build.root( "../sweet_build_tool.xcodeproj" );
    };
    zero_brane_studio = {
        mobdebug = build.switch { build.operating_system();
            macosx = "/Applications/ZeroBraneStudio.app/Contents/ZeroBraneStudio/lualibs/mobdebug/mobdebug.lua";
            windows = "C:\\Program Files (x86)\\ZeroBraneStudio\\lualibs\\mobdebug\\mobdebug.lua";
        };
    };
};

build.default_buildfiles {
    "boost/boost.build",
    "lua/lua.build",
    "sweet/assert/assert.build",
    "sweet/build_tool/build_tool.build",
    "sweet/cmdline/cmdline.build",
    "sweet/error/error.build",
    "sweet/lua/lua.build",
    "sweet/fs/fs.build",
    "sweet/persist/persist.build",
    "sweet/process/process.build",
    "sweet/rtti/rtti.build",
    "sweet/traits/traits.build",
    "unit/unit.build"
};

-- Targets built when building from the root directory and as targets when
-- generating XCode projects and Visual Studio solutions.
build.default_targets {
    "sweet/build_tool/build",
    "sweet/build_tool/build_hooks",
    "sweet/build_tool/build_tool_test"
};
