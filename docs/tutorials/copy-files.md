---
layout: page
title: Copy Files
parent: Tutorials
nav_order: 1
---

A simple example that copies the files `{bar,baz,foo}.in` to `output/{bar,baz,foo}.out` when the input files change or the output files don't exist:

The build script, *forge.lua*, that sets up the build is the following:

~~~lua
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
~~~

The call `require 'forge'` loads and initializes the main Forge module returning a Lua table that represents the module.

The call made on the returned Forge module creates a toolset that contains the settings for a build.  In this case the variable `output` is set to the directory to copy files to.

The call `toolset:Copy '${output}/%1.out'` is a pattern element that creates targets that copy their dependencies to `${output}/%1.out` where `%1` is replaced with the path to the dependency without its extension.

String substitution using variables from the toolset is carried out at the time the target is created so that `${output}` is replaced with the value that `output` was set to when the toolset was created.  Any variable can be set in a toolset and referenced later when specifying targets and dependencies.  There is nothing special about the name `output`.

The call made on the returned target adds `bar.in`, `baz.in`, and `foo.in` as dependencies.  The literal strings are converted into targets but explicit targets that build `bar.in` etc from other source files could have been used instead of literal strings.

The call to `toolset:all {}` adds the target that copies the file to the special "all" target for the working directory.  This "all" target is the target built by default by Forge when it is invoked from that working directory.

Because building a target implies building its dependencies adding targets as dependencies of the "all" target means that they're built by default when building that working directory.

Adding "all" targets from sub-directories to the "all" target of the root directory is the recommended way to specify targets that are built by default when building a project.

## Copy Rule

The `Copy` rule is defined as follows:

~~~lua
local Copy = PatternRule( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
~~~

The call to `PatternRule()` creates a rule that generates targets from patterns.  Behind the scenes this function creates a callable table that generates output targets by pattern matching and replacing the filename of each dependency passed to it.

The definition of `Copy.build()` defines the actions carried out when targets created with this rule are built.  Here the destination file is removed and copied from the source file.  In general any actions can be carried out from a build function including executing external processes.

Executing external processes is a parallel operation.  The build functions are called in separate Lua coroutines that yield on calls to `execute()`.  The yield suspends the coroutine until the executed process completes.  Other coroutines continue to execute to process as much of the dependency graph as possible.
