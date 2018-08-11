
-- Load the build module and `Copy` target prototype used to express copied
-- files in the Lua-based DSL.
require 'forge';
require 'forge.Copy';

-- Initialize the build with default settings before use.  Project 
-- specific settings can be passed in to override the defaults.
forge:initialize();

-- Use the Lua-based DSL to create a dependency graph that copies `foo.in` to
-- `foo.out` when `foo.in` is newer or `foo.out` doesn't exist.
forge:all {
    forge:Copy 'foo.out' {
        'foo.in'
    };    
};
