---
layout: page
title: Writing Toolsets
parent: Getting Started
nav_order: 4
---

- TOC
{:toc}

Toolsets add support for external tools by providing target prototypes and settings that are used to define a dependency graph of targets that represents the build of a project.

For example the *cc* toolset supports C and C++ compilation, the experimental *android* toolset builds Android packages, and the experimental *xcode* toolset builds macOS and iOS app bundles.

### Using Toolsets

Load toolsets as plain Lua modules using `require()`.  The returned value is actually a toolset prototype that can be called to create toolsets for a particular tool (e.g. a C/C++ compiler toolchain) to define the build.

Create a toolset by calling a toolset prototype.  The single, optional parameter to this call is a table containing settings to override any defaults provided by the toolset.  The call is often chained onto the require call and uses Lua's syntactic sugar to omit parentheses in function calls as follows:

~~~lua
local cc = require 'forge.cc' {
    identifier = 'cc_${platform}_${architecture}';
    platform = operating_system();
    bin = root( ('%s/bin'):format(variant) );
    lib = root( ('%s/lib'):format(variant) );
    obj = root( ('%s/obj/cc_%s_x86_64'):format(variant) );
    -- ...
};
~~~

More complex builds may load toolset prototypes into variables and use them explicitly to create toolsets to build for multiple platforms and/or architectures.

Toolsets that are created with `identifier` in their settings are added to the list of toolsets using the interpolated value of `identifier` as their unique identifier.  These toolsets are then available from within buildfiles using the `toolsets()` function.

The toolset is also returned from the toolset prototype call for use within the root build script.  Usually this is to provide conditional configuration that is inconvenient to pass in the settings.

See each individual toolset's documentation for exactly which target protoypes and settings are provided and expected.

### Writing Toolsets

Create a toolset prototype by calling `forge:ToolsetPrototype()`, passing a short identifier as the sole parameter.  This returns a table on which to define the functions provided by the toolset.  Return the toolset prototype from the end of the Lua script.

Any functions that are needed during definition of the dependency graph can be defined on the toolset prototype.  They will then be available from the toolset from the root build script and any loaded buildfiles.

Three functions that can be overridden to have the toolset prototype configure and initialize toolsets that are created from it are `Toolset.configure()`, `Toolset.validate()`, and `Toolset.initialize()`.

`Toolset.configure()` is called only when a toolset is created and there isn't a field in the local settings for the target prototype.  The local settings field is named after the short identifier that is passed to create the target prototype.  The call to `Toolset.configure()` is expected to perform one-off auto-detection of installed tools.

`Toolset.validate()` is called for every toolset that is created.  It should return true when the configuration in local settings is still correct.  Checking that any configured paths to tools are still valid is usually sufficient.  Returning false will quietly ignore the rest of the toolset creation.  If lacking the toolset is a fatal error then this function should assert or raise an error.

`Toolset.initialize()` is called for every toolset that is created and that successfully configures and validates.  It should setup target prototypes and any settings that need to be setup dynamically at runtime (e.g. variant dependent settings not suitable for storing in local settings).

The identifier passed to `forge:ToolsetPrototype()` identifies the block of per-machine settings returned by the `Toolset.configure()` function and stored in the *local_settings.lua* file.  The identifier must be unique within your build.  The configured values are shared between all variants; values that are different between variants should be computed in `Toolset.initialize()` which runs for each toolset in each build.

### Target Prototypes

Target prototypes associate targets with the Lua functions that determine what happens when a target is created, when dependencies are added, and when targets are built and cleaned.

Target prototypes are to targets as classes are to objects in languages like C++, C#, and Java.  If you're familiar with dynamic, prototype based languages like Lua or JavaScript then target prototypes are literally the prototypes that define the behavior of targets.

Create a prototype by calling `forge:TargetPrototype()` passing a descriptive identifier.  Override `create()`, `depend()`, `build()`, and/or `clean()` to provide custom behavior during target creation, adding dependencies, building, and cleaning respectively.

Targets without target prototypes are valid.  These targets are typically source files that aren't updated by the build system and so need to be tracked in order to determine when intermediate files that depend on them are outdated but don't need any custom behaviors.

The `Directory` target prototype is a good example that overrides the default behaviour for create, build, and clean actions:

~~~lua
local Directory = forge:TargetPrototype( 'Directory' );

function Directory.create( toolset, identifier )
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

The target prototype is created with a call to `forge:TargetPrototype()` passing `'Directory'` as the identifier for the target prototype.  The identifier only provides debug information during the dump of the dependency graph and so is only required to be informative.

The create function is called when the `Directory` prototype is used to create a target, e.g. when `toolset:Directory(identifier)` is called from a buildfile.  The `toolset` and `identifier` arguments from this call are forwarded through to `Directory.create()` and the target prototype, `Directory`, is forwarded into the third parameter.

The build function is called when running a build and the directory is determined to be outdated.  This simply uses the built-in `mkdir()` function to create the directory.  The `toolset` parameter is the same toolset that was used to create the directory target.  The `target` is the target that represents the directory that needs to be built.

The clean function is called when running the clean command.  The clean function here does nothing because the default clean behavior of deleting a file doesn't work on the directory.  The directory could be removed here with a call to `rmdir()` but that is often problematic if the directory is not empty.

The `Directory` prototype is returned at the end of the script.  This is the value that is returned from the `require()` call that is used to load the target prototype and store it into a toolset (e.g. `toolset.Directory = require 'forge.Directory';`).

Directories don't often appear directly in buildfiles.  Usually they are implicitly created and added as ordering dependencies of other built files.  That way any directories that files will be built into are created before files are built to them.

### Create

~~~lua
function Target.create( toolset, identifier, target_prototype )
~~~

The `create()` function is called whenever a target for that target prototype is created by calling the target prototype table returned from `build.TargetPrototype()`.

The parameters passed are the toolset that the target is being created with, the identifier that was specified, and the target prototype that was used to create the target (possibly nil).

### Depend

~~~lua
function Target.depend( toolset, target, dependencies )
~~~

The `depend()` function is called whenever a call is made on a target for that target prototype.  Typically this captures the use of calls of the form *target* **{** *dependencies* **}** to add dependencies or further define a target.

The parameters passed are the toolset that the target was created with, the target itself, and the dependencies that were passed in.

### Build

~~~lua
function Target.build( toolset, target )
~~~

The `build()` function is called whenever an outdated target is visited as part of a build traversal.  The function should carry out whatever actions are necessary to build the file(s) that it represents up to date.

The parameters passed in are the toolset that the target was created with and the target itself.

### Clean

~~~lua
function Target.clean( toolset, target )
~~~

The `clean()` function is called whenever a target is visited as part of a clean traversal.  The function should carry out whatever actions are necessary to remove files that were generated during a build traversal.

Default behavior when visiting a cleanable target is to remove any files that the target is bound to.  Custom clean behavior is only needed if removing all of the built files is not desired.

The parameters passed in are the toolset that the target was created with and the target itself.

### File Prototypes

~~~lua
function forge:FilePrototype( identifier )
~~~

Create target prototypes that build a single output file from zero or more input files with the `forge:FilePrototype()` prototype function.

The `forge:FilePrototype()` function is a special short-hand that provides a create function on the target prototypes that it returns.  That create function creates a target in the same way as the directory example above adding the directory that contains the file as an ordering dependency.

### Pattern Prototypes

~~~lua
function forge:PatternPrototype( identifier, replacement_modifier, pattern )
~~~

Create target prototypes that generate an output file per input file using pattern matching and replacement to generate identifiers for the output files.

The `forge:PatternPrototype()` function is a special short-hand that provides a proxy create function that generates output and input targets as dependencies are added in the build script.

The `Copy` prototype provided by *Forge* is a very simple pattern-based target prototype that only defines a build action:

~~~lua
local Copy = forge:PatternPrototype( 'Copy' );

function Copy.build( toolset, target )
    rm( target );
    cp( target, target:dependency() );
end

return Copy;
~~~

### Group Prototypes

~~~lua
function forge:GroupPrototype( identifier, replacement_modifier, pattern )
~~~

Create target prototypes that generate an output file per input file using a single invocation of a tool to build multiple files at once.  For example Microsoft's Visual C++ compiler will compile multiple source files at once.

The `forge:GroupPrototype()` function is a special short-hand that provides a proxy create function that generates targets as dependencies are added in the build script.

For example the `Cxx` target prototype used by the Microsoft Visual C++ module to compile C++ is defined as follows:

~~~lua
local Cxx = forge:GroupPrototype( 'Cxx', msvc.object_filename );
Cxx.language = 'c++';
Cxx.build = msvc.compile;
toolset.Cxx = Cxx;
~~~
