
-- Initialize Forge and load any existing dependency graph.
local forge = require( 'forge' ):load();

-- Configure a toolset with the output directory to *output*
-- relative to the root directory that contains *forge.lua*.
local toolset = forge.Toolset() {
    output = root( 'output' );
};

-- Use the Lua-based DSL to create a dependency graph that copies
-- the input files to the output directory with the *.out*
-- extension when the input files change or the output
-- files don't exist.
toolset:all {
    toolset:Copy '${output}/%1.out' {
        'bar.in';
        'baz.in';
        'foo.in';
    };    
};
