
local forge = require 'forge';
local toolset = forge.Toolset();
toolset:install( 'forge.cc' );

TestSuite {
    transitive_dependencies_found = function()
        local exe = toolset:Executable 'exe' {
            'foo';
            'baz';
        };
        local foo = toolset:StaticLibrary 'foo' {
            'bar';
        };
        local bar = toolset:StaticLibrary 'bar' {
            'baz';
        };
        local baz = toolset:StaticLibrary 'baz' {            
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
