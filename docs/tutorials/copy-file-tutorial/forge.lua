
-- Initialize Forge and configure the output directory to *output* relative
-- to the root directory that contains *forge.lua*.
local forge = require 'forge' {
	output = forge:root( 'output' );
};

-- Use the Lua-based DSL to create a dependency graph that copies `foo.in` to
-- `output/foo.out` when `foo.in` is newer or `output/foo.out` doesn't exist.
forge:all {
    forge:Copy '${output}/foo.out' {
        'foo.in'
    };    
};
