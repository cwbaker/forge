
-- Load the build module containing defaults, initializations, utility
-- functions and targets.
require "build";

-- Define the `Copy` target prototype that expresses a copied file in the
-- dependency graph.  This makes the callable `Copy` target prototype 
-- available in the Lua-based DSL used to specify dependency graphs.
local Copy = build:TargetPrototype( 'Copy' );

function Copy.create( build, settings, identifier )
    return build:File( identifier, Copy );
end

function Copy.build( build, target )
    build:rm( target );
    build:cp( target, target:dependency() );
end

-- Initialize the build with default settings before use.  Project 
-- specific settings can be passed in to override the defaults.
build:initialize();

-- Use the Lua-based DSL to create a dependency graph that copies `foo.in` to
-- `foo.out` when `foo.in` is newer or `foo.out` doesn't exist.
build:all {
    build:Copy 'foo.out' {
        'foo.in'
    };    
};
