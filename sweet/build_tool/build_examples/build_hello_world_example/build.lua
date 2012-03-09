
package.path = root("../../../build/lua/?.lua")..";"..root("../../../build/lua/?/init.lua");
require "build";

setup {};

Executable {
    id = "hello_world";
    Cc {
        "hello_world.cpp"
    }
}

build {};
