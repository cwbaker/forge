
-- Initialize Forge and configure the output directory to *output* relative
-- to the root directory that contains *forge.lua*.
local toolset = require 'forge' {
	output = root( 'output' );
};

-- Use the Lua-based DSL to create a dependency graph that copies the input
-- files to the output directory with the *.out* extension whenever the input
-- files are newer or the output files don't exist.
toolset:all {
    toolset:Copy '${output}/%1.out' {
        'bar.in';
        'baz.in';
        'foo.in';
    };    
};
