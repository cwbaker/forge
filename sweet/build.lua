
command  = command or "build";
platform = platform or "msvc";
source = source or "";
target = target or "";
variant = variant or "debug";
version = version or os.date( "%Y.%m.%d %H:%M:%S "..platform.." "..variant );
jobs = jobs or 4;

package.path = root("build/lua/?.lua")..";"..root("build/lua/?/init.lua");
require "build";
require "build/boost";

setup {
    bin = root( "../bin" );
    lib = root( "../lib" );
    obj = root( "../obj" );
    include_directories = {
        root( ".." ),
        "C:/boost/include/boost-1_43"
    };
    library_directories = {
        "C:/boost/lib"
    };
}

buildfile "assert/assert.build";
buildfile "atomic/atomic.build";
buildfile "build_tool/build_tool.build";
buildfile "cmdline/cmdline.build";
buildfile "error/error.build";
buildfile "lua/lua.build";
buildfile "path/path.build";
buildfile "persist/persist.build";
buildfile "pointer/pointer.build";
buildfile "process/process.build";
buildfile "rtti/rtti.build";
buildfile "thread/thread.build";
buildfile "traits/traits.build";
buildfile "unit/unit.build";

AsciiDoc {
    id = "build.html";
    "build.txt"
}

build {};
