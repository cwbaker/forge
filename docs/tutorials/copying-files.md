---
layout: page
title: Copying Files
parent: Tutorials
nav_order: 1
---

A simple example that copies the files `{bar,baz,foo}.in` to `output/{bar,baz,foo}.out` if the input files have changed since they were last built or the output files don't exist:

The root build script, *forge.lua*, that sets up the build is the following:

~~~lua
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
~~~

The call `require 'forge'` loads and initializes the main Forge module returning a Lua table that represents the module.

The call made on the returned Forge module creates a toolset that contains the settings for a build.  In this case the variable `output` is set to the directory to copy files to.

The call `toolset:Copy '${output}/%1.out'` is a pattern element that creates targets that copy their dependencies to `${output}/%1.out` where `%1` is replaced with the path to the dependency without its extension.

String substitution using variables from the toolset is carried out at the time the target is created so that `${output}` is replaced with the value that `output` was set to when the toolset was created.  Any variable can be set in a toolset and referenced later when specifying targets and dependencies.  There is nothing special about the name `output`.

The call made on the returned target adds `foo.in` as a dependency.  The literal string is implicitly converted into a target named `foo.in` but any target could have been created and passed in its place.

The call to `toolset:all {}` adds the target that copies the file to the special "all" target for the working directory.  This "all" target is the target built by default by Forge when it is invoked from that working directory.

Because building a target implies building its dependencies adding targets as dependencies of the "all" target means that they're built by default when building that working directory.

Adding "all" targets from sub-directories to the "all" target of the root directory is the recommended way to specify targets that are built by default when building a project.

Running `forge -r . dependencies` in the *copy-files-tutorial* directory displays the dependency graph generated to copy the files:

~~~bash
.../copy-files-tutorial> forge -r . dependencies 

'all' OtFScb 9999-99-99 99:99:99 0 
    Copy 'bar.out' OtFSCb 9999-99-99 99:99:99 a8d10b8e43f83872 1970-01-01 12:00:00
        >'output/bar.out'
        'bar.in' OTFScb 2019-07-29 13:05:39 a8d10b8e43f83872 2019-07-29 13:05:39
            >'bar.in'
        Directory 'output' OtFSCb 9999-99-99 99:99:99 a8d10b8e43f83872 *1970-01-01 12:00:00
            >'output'
    Copy 'baz.out' OtFSCb 9999-99-99 99:99:99 a8d10b8e43f83872 1970-01-01 12:00:00
        >'output/baz.out'
        'baz.in' OTFScb 2019-07-29 13:05:39 a8d10b8e43f83872 2019-07-29 13:05:39
            >'baz.in'
        Directory 'output' OtFSCb 9999-99-99 99:99:99 a8d10b8e43f83872 *1970-01-01 12:00:00
            >'output'
    Copy 'foo.out' OtFSCb 9999-99-99 99:99:99 a8d10b8e43f83872 1970-01-01 12:00:00
        >'output/foo.out'
        'foo.in' OTFScb 2019-07-29 13:05:39 a8d10b8e43f83872 2019-07-29 13:05:39
            >'foo.in'
        Directory 'output' OtFSCb 9999-99-99 99:99:99 a8d10b8e43f83872 *1970-01-01 12:00:00
            >'output'
~~~

## Copy Prototype

- The `Copy` prototype, the "rule" in parlance common to other build tools, is defined as follows:

~~~lua
local Copy = forge:FilePrototype( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
~~~

The call to `forge:FilePrototype()` creates a target prototype that generates files.  Behind this scenes this function is creating a plain target prototype and adding a create function that creates a target that generates a file, is cleanable, and that creates the directory that contains it if necessary.

The definition of `Copy.build()` defines the actions carried out when targets created with this prototype are outdated and need to be built.  Here the destination file is removed and copied again from the source file but, in general, any actions can be carried out including executing external processes.

Executing external processes is a parallel operation.  The Lua coroutine in which the call to `execute()` is made yields until the executed process completes while new coroutines are spawned to process as much of the dependency graph as possible.

## Copy Pattern Element

From the bottom of *Toolset.lua* where `Copy` is setup as an element available to all toolsets.

~~~lua

Toolset.Copy = forge:PatternElement( require('forge.Copy') );

~~~

The call to `forge:PatternElement()` creates and returns a function that can be used in buildfiles to generate targets using pattern matching and replacement.
