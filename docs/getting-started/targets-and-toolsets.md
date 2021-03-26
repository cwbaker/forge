---
layout: page
title: Targets and Toolsets
parent: Getting Started
nav_order: 4
---

- TOC
{:toc}

Support new tools by creating target prototypes that define the behavior of targets built with those tools.  Bundle related target prototypes together in a toolset prototype to configure and initialize them for use in toolsets.

Target and toolset prototypes are written as Lua modules that return the prototype that they define.  The result of a `require()` call to load the prototype is then assigned directly to a variable within a toolset (for target prototypes) or used to create a new toolset or installed into an existing toolsets (for toolset prototypes).

Set the `require()` search path by assigning to `package.path` at the top of each project's root build script.  Use `root()` and `executable()` to generate paths relative to the project root directory and Forge executable respectively.

### Target Prototypes

Target prototypes provide targets with behaviors that determine what happens when a target is created, when dependencies are added, and when a target is built or cleaned.

Three functions can be overridden to interact with target creation and dependency addition:

- `Target.create()` is called whenever a target prototype is called to create a new target.  This captures calls of the form *TargetPrototype* **'** *identifier* **'** to create new targets from buildfiles.

- `Target.depend()` is called when a target of the target prototype is called to add dependencies.  This captures calls of the form *target* **{** *dependencies* **}** that add dependencies or further define a target.

Two functions can be overridden to interact with build and clean actions during a traversal:

- `Target.build()` is called whenever an outdated target is visited as part of a build traversal.  The function build the file(s) represented by the target.

- `Target.clean()` is called whenever a target is visited as part of a clean traversal.  The function should remove files that were generated during a build.  The default clean action for targets marked as cleanable is to delete the file(s) represented by the target and a custom function is only needed if this behavior isn't correct.

For target prototypes that need custom create and depend functions call `TargetPrototype()` to create and return a raw target prototype on which at least a create function must be defined.

For example here is the target prototype `Directory` defined as a core part of Forge and used to ensure that directories for output files exist before those files are created:

~~~lua
local Directory = TargetPrototype( 'Directory' );

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

For targets that build to single files or multiple files based on a pattern or group there are convenience functions to create target prototypes with create and depend functions defined:

- `FilePrototype()` returns a target prototype with create and depend functions that support building one or more source files to a single file.

- `PatternPrototype()` returns a target prototype with create and depend functions that support building multiple source files to multiple files (with a one-to-one mapping) where the destination filename is derived from a pattern matching the source filename.

- `GroupPrototype()` returns a target prototype with create and depend functions that support building multiple source files to multiple files in groups where the destination filename is derived from a pattern matching the source filename.  For example the Visual C++ compiler accepts multiple C++ source files in one invocation and it is useful to group these together.

For example the `Copy` target prototype that copies files:

~~~lua
local Copy = PatternPrototype( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
~~~

Target prototypes defined with `FilePrototype()`, `PatternPrototype()`, or `GroupPrototype()` provide two more functions that can be overridden to interact with target creation:

- `Target.identify()` is called before a target is created to determine the identifier and filename to use for the target.  It is passed the toolset and base identifier and should return identifier and filename.

- `Target.created()` is called after a target has been created.  This allows for extra processing immediately after a target has been created.  Useful for adding explicit dependencies on excutables used to build those targets.

For example the following target prototype adds support for compiling grammars into parse tables with that *lalrc* tool by defining the target prototype `Lalrc` in the `lalr` toolset prototype.  See [.../lalr/Lalrc.lua](https://github.com/cwbaker/lalr/blob/main/lalr/forge/lalr/Lalrc.lua):

~~~lua
local Lalrc = PatternPrototype( 'Lalrc' );

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

### Toolset Prototypes

Toolset prototypes provide toolsets with behaviors that allow for toolsets to perform one-off auto-detection of tools installed on a machine, validation of configuration on each run, and initialization of target prototypes and settings that happens for each toolset.

Create a toolset prototype by calling `ToolsetPrototype()` passing a short identifier as the sole parameter.  This returns a table on which to define the functions provided by the toolset.  Return the toolset prototype from the end of the Lua script that defines it.

Three functions can be overridden to interact with Forge's configuration and initialization of toolsets:

- `Toolset.configure()` is called only when a toolset is created and there isn't a field in the local settings for the target prototype.  The local settings field is named after the short identifier that is passed to create the target prototype.  The call to `Toolset.configure()` is expected to perform one-off auto-detection of installed tools.

- `Toolset.validate()` is called for every toolset that is created.  It should return true when the configuration in local settings is still correct.  Checking that any configured paths to tools are still valid is usually sufficient.  Returning false will quietly ignore the rest of the toolset creation.  If lacking the toolset is a fatal error then this function should assert or raise an error.

- `Toolset.initialize()` is called for every toolset that is created and that successfully configures and validates.  It should setup target prototypes and any settings that need to be setup dynamically at runtime (e.g. variant dependent settings not suitable for storing in local settings).
 
Other functions can be defined on the toolset prototype as needed.  These functions are then available on toolsets using that toolset prototype from the root build script and any loaded buildfiles.

For example the following build script defines the `lalr` toolset prototype in [.../lalr/init.lua](https://github.com/cwbaker/lalr/blob/main/lalr/forge/lalr/init.lua):

~~~lua
local lalr = ToolsetPrototype( 'lalr' );

function lalr.configure( toolset, lalr_settings )
    return {
        lalrc = lalr_settings.lalrc or '${bin}/lalrc';
    };
end

function lalr.initialize( toolset )
    toolset.Lalrc = require( 'forge.lalr.Lalrc' );
    return true;
end

return lalr;
~~~

The `lalr` toolset prototype is then used by installing it into a toolset used to build C/C++ source.  For example the `forge.cc` toolset prototype is required and called to create the toolset `cc` which the `forge.lalr` toolset prototype is then required and installed into:

~~~lua
local cc = require 'forge.cc' {
    -- Settings to initialize cc and lalr toolsets here...
};

local lalr = require 'forge.lalr';
cc:install( lalr );
~~~
