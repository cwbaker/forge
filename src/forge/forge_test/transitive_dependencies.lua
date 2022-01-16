
require 'forge';
local cc = require 'forge.cc' {};

TestSuite {
    transitive_dependencies_found = function()
        local exe = cc:Executable 'exe' {
            'foo';
            'baz';
        };
        local foo = cc:StaticLibrary 'foo' {
            'bar';
        };
        local bar = cc:StaticLibrary 'bar' {
            'baz';
        };
        local baz = cc:StaticLibrary 'baz' {            
        };

        CHECK( exe:dependency(1) == foo );
        CHECK( exe:dependency(2) == baz );
        CHECK( exe:dependency(3) == nil );

        prepare( exe );

        CHECK( exe:dependency(1) == foo );
        CHECK( exe:dependency(2) == bar );
        CHECK( exe:dependency(3) == baz );
        CHECK( exe:dependency(4) == nil );
    end;
};
