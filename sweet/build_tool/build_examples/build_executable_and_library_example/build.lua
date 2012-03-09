
package.path = root("../../../build/lua/?.lua")..";"..root("../../../build/lua/?/init.lua");
require "build";

setup {};

Executable {
    id = "executable";    
    dependencies = {
        "library"
    };
    Cc {
        "executable.cpp"
    };
};

Library {
    id = "library";
    Cc {
        "library.cpp"
    };
};

build {};
