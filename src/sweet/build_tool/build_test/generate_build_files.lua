
local build_lua_source = [[
command  = command or "build";
platform = platform or "msvc";
source = source or "";
target = target or "";
variant = variant or "debug";
version = version or os.date( "%%Y.%%m.%%d %%H:%%M:%%S "..platform.." "..variant );
jobs = jobs or 4;

package.path = root("../../../build/lua/?.lua")..";"..root("../../../build/lua/?/init.lua");
require "build";

setup {
    include_directories = {
        root(),
    };
};

%s

build {};

]];

local library_buildfile_source = [[
Library {
    id = "lib_%s";
    Cc {%s
    };
};
]];

getmetatable("").__mod = function( format, args )
    if args then
        if type(args) == "table" then
            return string.format( format, unpack(args) );
        else
            print( args );
            return string.format( format, args );
        end
    else
        return format;
    end
end

function generate_build_files( libraries, classes )
    local function class_list( classes )
        local class_list = ""; 
        for i = 1, classes do
            class_list = class_list..string.format( "\n        \"class_%d.cpp\",", i );
        end
        return class_list;
    end

    local function buildfile_list( libraries )
        local buildfile_list = "";
        for i = 1, libraries do
            buildfile_list = buildfile_list..string.format( "buildfile \"lib_%d/lib_%d.build\";\n", i, i );
        end
        return buildfile_list;
    end

    local build_lua = io.open( root("build/build.lua"), "wb" );
    assert( build_lua );
    build_lua:write( build_lua_source % buildfile_list(libraries) );
    build_lua:close();
    build_lua = nil;
    
    for i = 1, libraries do
        local buildfile = io.open( root("build/lib_%d/lib_%d.build" % {i, i}), "wb" );
        assert( buildfile );
        buildfile:write( library_buildfile_source % {i, class_list(classes)} );
        buildfile:close();
        buildfile = nil;
    end    
end
