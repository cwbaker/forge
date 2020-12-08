
local cc = require 'forge.cc' {};

local executable = cc:Executable 'executable' {
    cc:StaticLibrary 'foo' {
        'baz';
        cc:StaticLibrary 'bar' {
            cc:StaticLibrary 'baz';
        };
    };
};

local libraries = executable:find_transitive_libraries();
CHECK( libraries[1] == find_target('foo') );
CHECK( libraries[2] == find_target('bar') );
CHECK( libraries[3] == find_target('baz') );
