---
layout: page
title: Targets and Toolsets
parent: Getting Started
nav_order: 4
---

- TOC
{:toc}

Support new tools by creating rules that define the behavior of targets built with those tools.  Bundle related rules together in a module to configure and initialize them within toolsets.

Rules and modules are written as Lua modules that return the rule or module that they define.  The result of a `require()` call to load the module is then assigned to a variable within a toolset (for rules) or used to create a installed into one or more toolsets (for modules).

Set the `require()` search path by assigning to `package.path` at the top of each project's root build script.  Use `root()` and `executable()` to generate paths relative to the project root directory and Forge executable respectively.

### Rules

Rules provide targets with behaviors that determine what happens when a target is created, when dependencies are added, and when a target is built or cleaned.

Two functions can be overridden to interact with target creation and dependency addition:

- `Target.create()` is called whenever a rule is called to create a new target.  This captures calls of the form *Rule* **'** *identifier* **'** to create new targets from buildfiles.

- `Target.depend()` is called when a target of the rule is called to add dependencies.  This captures calls of the form *target* **{** *dependencies* **}** that add dependencies or further define a target.

Two functions can be overridden to interact with build and clean actions during a traversal:

- `Target.build()` is called whenever an outdated target is visited as part of a build traversal.  The function should build the file(s) represented by the target.

- `Target.clean()` is called whenever a target is visited as part of a clean traversal.  The function should remove files that were generated during a build.  The default clean action for targets marked as cleanable is to delete the file(s) represented by the target and a custom function is only needed if this behavior isn't correct.

For rules that need custom create and depend functions call `Rule()` to create and return a raw rule on which at least a create function must be defined.

For example here is the rule `Directory` defined as a core part of Forge and used to ensure that directories for output files exist before those files are created:

~~~lua
local Directory = Rule( 'Directory' );

function Directory.create( toolset, identifier )
    local identifier = toolset:interpolate( identifier );
    local target = toolset:Target( identifier, Directory );
    target:set_filename( target:path() );
    target:set_cleanable( true );
    return target;
end

function Directory.build( toolset, target )
    mkdir( target );
end

function Directory.clean( toolset, target )
end

return Directory;
~~~

For targets that build to single files or multiple files based on a pattern or group there are convenience functions to create rules with create and depend functions defined:

- `FileRule()` returns a rule with create and depend functions that support building one or more source files to a single file.

- `PatternRule()` returns a rule with create and depend functions that support building multiple source files to multiple files (with a one-to-one mapping) where the destination filename is derived from a pattern matching the source filename.

- `GroupRule()` returns a rule with create and depend functions that support building multiple source files to multiple files in groups where the destination filename is derived from a pattern matching the source filename.  For example the Visual C++ compiler accepts multiple C++ source files in one invocation and it is useful to group these together.

For example the `Copy` rule that copies files:

~~~lua
local Copy = PatternRule( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
~~~

Rules defined with `FileRule()`, `PatternRule()`, or `GroupRule()` provide two more functions that can be overridden to interact with target creation:

- `Target.identify()` is called before a target is created to determine the identifier and filename to use for the target.  It is passed the toolset and base identifier and should return identifier and filename.

- `Target.created()` is called after a target has been created.  This allows for extra processing immediately after a target has been created.  Useful for adding explicit dependencies on excutables used to build those targets.

For example the following rule adds support for compiling grammars into parse tables with that *lalrc* tool by defining the rule `Lalrc` in the `lalr` module.  See [.../lalr/Lalrc.lua](https://github.com/cwbaker/lalr/blob/main/lalr/forge/lalr/Lalrc.lua):

~~~lua
local Lalrc = PatternRule( 'Lalrc' );

function Lalrc.created( toolset, target )
    local settings = toolset.settings;
    local lalrc = toolset:interpolate( settings.lalr.lalrc );
    target:add_dependency( toolset:Target(lalrc) );
end

function Lalrc.build( toolset, target )
    local lalrc = target:dependency( 1 );
    local source = target:dependency( 2 );
    local filename = target:filename();
    printf( leaf(filename) );
    system( lalrc,
        ('lalrc -o "%s" "%s"'):format(filename, source)
    );
end

return Lalrc;
~~~

The `created()` function adds a dependency on the *lalrc* executable.  The executable is built from source and this dependency ensures the executable is built before it is used.  The `build()` function brings the parse table up to date by executing *lalrc* passing the input and output filenames on the command line.

### Modules

Modules provide toolsets with behaviors that allow for toolsets to perform one-off auto-detection of tools installed on a machine, validation of configuration on each run, and initialization of rules and settings that happens for each toolset.

Create a module by creating a Lua table that contains an `install()` function.  The `install()` function accepts a toolset in which the module should set rules to create targets for the tools it supports and default values for any settings that haven't already been set in the toolset.  Return the module from the end of the Lua script that defines it.

For example the following build script defines the `lalr` module in [.../lalr/init.lua](https://github.com/cwbaker/lalr/blob/main/lalr/forge/lalr/init.lua):

~~~lua
local lalr = ToolsetPrototype( 'lalr' );

function lalr.configure( toolset, lalr_settings )
    return {
        lalrc = lalr_settings.lalrc or '${bin}/lalrc';
    };
end

function lalr.initialize( toolset )
    local settings = toolset:configure_once( 'lalr', lalr.configure );
    assert( exists(settings.lalrc) );
    toolset.Lalrc = require( 'forge.lalr.Lalrc' );
    return true;
end

return lalr;
~~~

The `lalr` module is then used by installing it into a toolset used to build C/C++ source.  For example the `forge.cc` module is required and called to create the toolset `cc` which the `forge.lalr` module is then required and installed into:

~~~lua
local forge = require( 'forge.cc' ):load( variant );

local cc = forge.Toolset 'cc_${platform}_${architecture}' {
    -- Settings to initialize cc and lalr toolsets here...
};

local lalr = require 'forge.lalr';
cc:install( lalr );
~~~
