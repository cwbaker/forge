
package.path = root("../../../build/lua/?.lua")..";"..root("../../../build/lua/?/init.lua");
require "build";
require "build/msvc";

function initialize()
    local settings = build.initialize {};
    msvc.initialize( settings );
end

function buildfiles()
    Executable {
        id = "hello_world";
        Source {
            "hello_world.cpp"
        };
    }
end
